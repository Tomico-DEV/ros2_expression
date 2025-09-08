// Copyright 2025 TomicoDEV
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 *                    -
 *   /\                 \       _________________
 *  //\\                 \     /                 \
 * //  \\          <<<    |   |  ROS2 EXPRESSION  |
 *             <<<<        |   \ ________________/
 *          <<             |   |/ 
 *             <<<<        |
 *                 <<<    |
 *                       /
 *                      /
 *                    -
 * \author TomicoDEV
 * \file synthesis_stream.hpp
 * \brief SynthesisStream, a AudioStream that synthesizes audio upon construction and
 *        provides them as audio is played back
 */

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

// SFML
#include <SFML/Audio.hpp>

// concurrent queue
#include "concurrentqueue.h"

// Voicevox
#include "voicevox_speaker/voicevox_utils.hpp"
#include "voicevox_speaker/voicevox.hpp"


namespace voicevox
{

class SynthesisStream : public sf::SoundStream
{
public:
  SynthesisStream(
    AudioQuery query,
    uint32_t style_id,
    std::shared_ptr<Voicevox> vv);

  ~SynthesisStream();

  /**
   * \brief Cancels the synthesis. Can't be undone
   */
  void cancel();

private:
  // called when more data is needed
  bool onGetData(Chunk & data) override;
  inline void onSeek(sf::Time) override {};  // unsupported
  void synthesize_(uint i);
  uint synthesize_ahead_(double buffer_time = 3.0);

  AudioQuery query_;
  uint32_t style_id_;
  std::shared_ptr<Voicevox> vv_;

  moodycamel::ConcurrentQueue<sf::SoundBuffer> wav_queue_;

  std::atomic_bool running_{true};
  std::atomic_bool synthesis_done_{false};
  // std::mutex queue_mutex_;
  std::jthread synthesis_thread_;
  // std::condition_variable synth_cv_;
};

}  // namespace voicevox
