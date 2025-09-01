#include "voicevox_speaker/synthesis_stream.hpp"

namespace voicevox
{

SynthesisStream::SynthesisStream(
    AudioQuery query, 
    uint32_t style_id,
    std::shared_ptr<Voicevox> vv
)
: query_ { query }, style_id_ { style_id }, vv_ { vv }
{  
    // synthesize first chunk
    synthesize(0);    

    if (wav_queue_.empty())
        throw std::runtime_error("Failed to sythesize first chunk!");
    
    sf::SoundBuffer& buf = wav_queue_.front();

    // initialize
    initialize(
        buf.getChannelCount(), 
        buf.getSampleRate(), 
        buf.getChannelMap()
    );

    // start synthesis thread
    synthesis_thread_ = std::jthread{
        [this]() {
            try 
            {
                const uint query_len = query_.get_query_length();
                for (uint i = 1; i < query_len and running_.load(); ++i)
                {
                    synthesize(i);
                }
                // finished synthesis
                synthesis_done_.store(true);
                synth_cv_.notify_all();
            } catch (...) {
                running_.store(false);
                synthesis_done_.store(true);
                synth_cv_.notify_all();
            }
        }
    };
}

SynthesisStream::~SynthesisStream()
{
    cancel();
}

void SynthesisStream::cancel()
{
    stop();

    running_.store(false);
    synth_cv_.notify_all();
    
    if (synthesis_thread_.joinable())
        synthesis_thread_.join();
}

bool SynthesisStream::onGetData(Chunk& data)
{
    std::unique_lock lock { queue_mutex_ };
 
    // Wait until a buffer is available, synthesis finished, or cancelled
    synth_cv_.wait(lock, 
        [&]() {
            return !wav_queue_.empty() 
                or synthesis_done_.load()
                or !running_.load(); 
        }
    );

    // queue is empty yet a notification was sent; we're done
    if (wav_queue_.empty())
        return false;

    auto& buf = wav_queue_.front();
    data.samples = buf.getSamples();
    data.sampleCount = buf.getSampleCount();

    // data is copied so safe to pop
    wav_queue_.pop();

    return true;
}

/**
 * \brief synthesize ith chunk of query and add to wav_queue_
 */
void SynthesisStream::synthesize(uint i)
{
    try
    {
        sf::SoundBuffer buf;
        
        {
            WavAudio wav { vv_->synthesize_chunk(query_, i, style_id_) };
            if (!buf.loadFromMemory(wav.get_data(), wav.get_size()))
                throw std::runtime_error(
                    "Failed to load synthesized wav from memory!"
                );
        }

        {
            // lock only when modifying the queue
            std::lock_guard lock { queue_mutex_ };
            wav_queue_.push(buf);
        }
        synth_cv_.notify_one();
    } catch (...) {
        running_.store(false);
        synth_cv_.notify_all();
    }
}

}