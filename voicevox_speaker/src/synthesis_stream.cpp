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
 * \file synthesis_stream.cpp
 * \brief SynthesisStream implementation
 * 
 * See header for details
*/

#include "voicevox_speaker/synthesis_stream.hpp"

namespace voicevox
{

/**
 * \brief helper function to join sound buffers
 * \param buffers reference to vector of buffers to join
 * \return concatenated SoundBuffer
 */
auto concatenate_buffers(const std::vector<sf::SoundBuffer> & buffers)
-> sf::SoundBuffer
{
  // safety check
  if (buffers.empty()) {
    throw std::runtime_error("No buffers to concatenate!");
  }

  uint channel_count = buffers.front().getChannelCount();
  uint sample_rate = buffers.front().getSampleRate();
  auto channel_map = buffers.front().getChannelMap();
  // compute total sample count
  size_t total_samples = 0;
  for (auto & buf : buffers) {
    total_samples += buf.getSampleCount();
  }

  std::vector<int16_t> samples;
  samples.reserve(total_samples);

  // append all buffer samples
  for (auto & buf : buffers) {
    const int16_t * buf_samples = buf.getSamples();
    samples.insert(
      samples.end(), buf_samples,
      buf_samples + buf.getSampleCount());
  }

  // create a new buffer
  sf::SoundBuffer result;
  if (!result.loadFromSamples(
    samples.data(), samples.size(), channel_count,
    sample_rate, channel_map))
  {
    throw std::runtime_error("Failed to load concatenated buffer!");
  }

  return result;
}

SynthesisStream::SynthesisStream(
  AudioQuery query, uint32_t style_id,
  std::shared_ptr<Voicevox> vv)
: query_{query}, style_id_{style_id}, vv_{vv}
{
  // synthesize first chunk
  uint initial_i = synthesize_ahead_(5.0);

  // initialize
  initialize(1, query_.get_sample_rate(), {sf::SoundChannel::Mono});

  running_.store(true);

  if (running_) {
    // start synthesis thread
    synthesis_thread_ = std::jthread{
      [this, initial_i]() {
        try {
          const uint query_len = query_.get_query_length();
          for (uint i = initial_i + 1; i < query_len && running_.load(); ++i) {
            synthesize_(i);
          };
          synthesis_done_.store(true);
        } catch (...) {
          running_.store(false);
          synthesis_done_.store(true);
        }
      }
    };
  }
}

SynthesisStream::~SynthesisStream()
{
  cancel();
}

void SynthesisStream::cancel()
{
  stop();

  running_.store(false);

  if (synthesis_thread_.joinable()) {
    synthesis_thread_.join();
  }
}

bool SynthesisStream::onGetData(Chunk & data)
{
  // Wait until a buffer is available, synthesis finished, or cancelled

  std::vector<sf::SoundBuffer> buffers;

  sf::SoundBuffer buf;
  while (wav_queue_.try_dequeue(buf)) {
    buffers.push_back(std::move(buf));
  }

  if (!buffers.empty()) {
    sf::SoundBuffer res_buf = concatenate_buffers(buffers);
    data.samples = res_buf.getSamples();
    data.sampleCount = res_buf.getSampleCount();
    return true;
  }

  // no data was dequeued
  if (synthesis_done_.load()) {
    return false;
  }
  // queue is empty yet a notification was sent; we're done

  // synthesis can't catch up -- return silence
  static std::vector<int16_t> silence(1024, 0);
  data.samples = silence.data();
  data.sampleCount = silence.size();

  return true;
}

/**
 * \brief synthesize ith chunk of query and add to wav_queue_
 */
void SynthesisStream::synthesize_(uint i)
{
  try {
    sf::SoundBuffer buf;

    {
      WavAudio wav{vv_->synthesize_chunk(query_, i, style_id_)};
      if (!buf.loadFromMemory(wav.get_data(), wav.get_size())) {
        throw std::runtime_error("Failed to load synthesized wav from memory!");
      }
    }

    wav_queue_.enqueue(std::move(buf));
  } catch (...) {
    running_.store(false);
  }
}

/**
 * \brief synthesize ahead until length of synthesized audio exceeded buffer time
 * \param buffer_time buffer time
 * \return index of chunk where synthesis ended
 */
uint SynthesisStream::synthesize_ahead_(double buffer_time)
{
  double cumul_time = 0.0;
  uint i = 0;
  for (; i < query_.get_query_length() && running_.load(); ++i) {
    cumul_time += query_.get_chunk_dur(i);
    synthesize_(i);
    if (cumul_time > buffer_time)
      break;
  }

  return i;
}

}  // namespace voicevox
