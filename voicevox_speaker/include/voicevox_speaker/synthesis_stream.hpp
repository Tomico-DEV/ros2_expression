#include "voicevox_speaker/voicevox_utils.hpp"
#include "voicevox_speaker/voicevox.hpp"

#include <SFML/Audio.hpp>

#include <atomic>
#include <condition_variable>
#include <queue>
#include <mutex>
#include <thread>

namespace voicevox
{

class SynthesisStream : public sf::SoundStream
{
public:
    SynthesisStream(
        AudioQuery query, 
        uint32_t style_id, 
        std::shared_ptr<Voicevox> vv
    );
    ~SynthesisStream();

    /**
     * \brief Cancels the synthesis. Can't be undone
     */
    void cancel();
    
private:
    // called when more data is needed
    bool onGetData(Chunk& data) override;
    inline void onSeek(sf::Time) override {}; // unsupported
    void synthesize(uint i);

    AudioQuery query_;
    uint32_t style_id_;
    std::shared_ptr<Voicevox> vv_;

    std::queue<sf::SoundBuffer> wav_queue_;

    std::atomic_bool running_ { true };
    std::atomic_bool synthesis_done_ { false };
    std::mutex queue_mutex_;
    std::jthread synthesis_thread_;
    std::condition_variable synth_cv_;
};

}