#include "voicevox_speaker/synthesis_stream.hpp"

#include <iostream>

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
    sf::SoundBuffer buf;
    WavAudio wav { vv_->synthesize_chunk(query_, 0, style_id_) };
    if (!buf.loadFromMemory(wav.get_data(), wav.get_size()))
        throw std::runtime_error(
            "Failed to load synthesized wav from memory!"
        );


    // initialize
    initialize(
        buf.getChannelCount(), 
        buf.getSampleRate(), 
        buf.getChannelMap()
    );

    std::cout << "SampleRate: " << buf.getSampleRate()
          << " Channels: " << buf.getChannelCount() << "\n";

    wav_queue_.enqueue(std::move(buf));

    uint initial_i = 0;
    /*
    if (5.0 - query_.get_chunk_len(0) > 0.0)
    {
        initial_i = synthesize_ahead(5.0 - query_.get_chunk_len(0));
    }
    */
    

    /*
    if (wav_queue_.size_approx() == 0)
    throw std::runtime_error("Failed to sythesize first chunk!");
    
    sf::SoundBuffer buf;
    if (!wav_queue_.try_dequeue(buf))
    throw std::runtime_error("Failed to dequeue first synthesized chunk!");
    */

    
    
    if (running_)
        // start synthesis thread
        synthesis_thread_ = std::jthread{
            [this, initial_i]() {
                try 
                {
                    const uint query_len = query_.get_query_length();
                    for (uint i = initial_i + 1; i < query_len and running_.load(); ++i)
                    {
                        synthesize(i);
                    }
                    // finished synthesis
                    std::cout << "Synthesis Complete\n";
                    synthesis_done_.store(true);
                } catch (...) {
                    running_.store(false);
                    synthesis_done_.store(true);
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
    
    if (synthesis_thread_.joinable())
        synthesis_thread_.join();
    
    std::cout << "Deiniting!\n";
}

bool SynthesisStream::onGetData(Chunk& data)
{
    std::cout << "Getting more data\n";
    // Wait until a buffer is available, synthesis finished, or cancelled
    
    // queue is empty yet a notification was sent; we're done
    sf::SoundBuffer buf;
    if (!wav_queue_.try_dequeue(buf))
    {
        if (synthesis_done_.load())
            return false;
        
        // synthesis can't catch up -- return silence
        static std::vector<int16_t> silence(1024, 0);
        data.samples     = silence.data();
        data.sampleCount = silence.size();

        std::cout << "Can't catch up!\n";
        return true;
    }

    constexpr size_t padding = 0.095 * 24000;

    data.samples = buf.getSamples() + padding;
    data.sampleCount = buf.getSampleCount() - padding;

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
            std::cout << "Synthesis done: " << i << "\n";
        }

        wav_queue_.enqueue(std::move(buf));
    } catch (...) {
        running_.store(false);
    }
}

uint SynthesisStream::synthesize_ahead(double buffer_time)
{
    double cumul_time = 0.0;
    uint i = 1;
    for (; i < query_.get_query_length() and running_.load(); ++i)
    {
        cumul_time += query_.get_chunk_len(i);
        synthesize(i);
        if (cumul_time > buffer_time)
            break;
    }

    return i;
}

}