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
 * \file voicevox_utils.hpp
 * \brief Utility classes and functions for voicevox speaker
 */

#pragma once

#include <stdexcept>
#include <string>
#include <filesystem>

#include <unordered_set>
#include <vector>

// JSON
#include <nlohmann/json.hpp>

// Rhubarb
#include "time/BoundedTimeline.h"
#include "core/Phone.h"

// Voicevox
#include "voicevox_core/voicevox_core.h"

namespace voicevox
{

/**
 * \brief convenience function for checking Voicevox function return codes
 * 
 * Throws a std::runtime if result is not VOICEVOX_RESULT_OK
 */
void throw_if_err(VoicevoxResultCode result);

// forward declare of synthesizer for AudioQuery
class Synthesizer;

/**
 * \brief c++ wrapper for Voicevox WAV
 */
class WavAudio
{
public:
  /**
   * \param wav_size size of wav file in memory
   * \param wav pointer to wav file in memory
   */
  explicit inline WavAudio(size_t wav_size, uint8_t * wav)
  : size_{wav_size}, data_{wav}
  {}
  /**
   * \brief free wav file in memory
   */
  inline ~WavAudio()
  {
    voicevox_wav_free(data_);
  }

  // no copying allowed
  WavAudio(const WavAudio &) = delete;
  // move allowed
  inline WavAudio(WavAudio && w) noexcept
  : size_{w.size_}, data_{w.data_}
  {
      w.size_ = 0;
      w.data_ = nullptr;
  }
  inline WavAudio& operator=(WavAudio&& w) noexcept {
    if (this != &w)
    {
      // free old resource
      voicevox_wav_free(data_);

      // steal new resource
      size_ = w.size_;
      data_ = w.data_;

      // leave w safe
      w.size_ = 0;
      w.data_ = nullptr;
    }

    return *this;
  }

  inline void * get_data()
  {
    return static_cast<void *>(data_);
  }
  inline size_t get_size()
  {
    return size_;
  }
private:
  size_t size_;
  uint8_t * data_;
};

typedef VoicevoxInitializeOptions InitializeOptions;
typedef VoicevoxOnnxruntime Onnxruntime;

/**
 * \brief RAII wrapper for Voicevox's LoadOrtOptions
 */
class LoadOrtOptions
{
public:
  /**
   * \param ort_file path to ort library
   */
  explicit inline LoadOrtOptions(const std::filesystem::path & ort_file)
  : option_{voicevox_make_default_load_onnxruntime_options()}
  {
    option_.filename = ort_file.c_str();
  }
  inline VoicevoxLoadOnnxruntimeOptions get() const
  {
    return option_;
  }
private:
  VoicevoxLoadOnnxruntimeOptions option_;
};

/**
 * \brief RAII wrapper for Voicevox's OpenJtalkRC
 */
class OpenJtalkAnalyzer
{
public:
  /**
   * \param dict_file path to dictionary file
   */
  explicit inline OpenJtalkAnalyzer(const std::filesystem::path & dict_file)
  : source_fpath_{dict_file.string()}
  {
    throw_if_err(
      voicevox_open_jtalk_rc_new(dict_file.c_str(), &open_jtalk_));
  }
  // copy constructor
  inline OpenJtalkAnalyzer(const OpenJtalkAnalyzer & obj)
  : source_fpath_{obj.source_fpath_}
  {
    throw_if_err(
      voicevox_open_jtalk_rc_new(source_fpath_.c_str(), &open_jtalk_));
  }
  inline ~OpenJtalkAnalyzer()
  {
    // free dictionary properly
    voicevox_open_jtalk_rc_delete(open_jtalk_);
  }
  inline OpenJtalkRc * get() const
  {
    return open_jtalk_;
  }

private:
  OpenJtalkRc * open_jtalk_;
  std::string source_fpath_;
};

/**
 * \brief RAII wrapper for Voicevox's VoiceModel
 */
class VoiceModel
{
public:
  /**
   * \param model_fpath path to voice model
   */
  explicit inline VoiceModel(std::filesystem::path model_fpath)
  {
    throw_if_err(
      voicevox_voice_model_file_open(model_fpath.c_str(), &model_));
  }
  inline ~VoiceModel()
  {
    voicevox_voice_model_file_delete(model_);
  }
  inline VoicevoxVoiceModelFile * get()
  {
    return model_;
  }

private:
  VoicevoxVoiceModelFile * model_;
};

class AudioQuery
{
public:
  /**
   * \param synth synthesizer to use for query generation
   * \param text text to turn into query
   * \param style_id style/speker id to use for query
   */
  AudioQuery(
    const Synthesizer & synth,
    std::string text,
    uint32_t style_id);

  /**
   * \brief get size of query
   * \return number of chunks
   */
  uint get_query_length() const;
  /**
   * \brief get ith chunk
   * \param i ith chunk
   * A 'chunk' is a collection of accent phrases taken from the original query
   * up until a phrase with a pause mora is found
   * \return chunk json 
   */
  nlohmann::json get_chunk(uint i) const;
  /**
   * \brief get underlying json object via const ref
   */
  const nlohmann::json& get() const;
  /**
   * \brief get duration of ith chunk in seconds
   * \param i ith chunk
   * \return duration of the chunk 
   */
  double get_chunk_dur(uint i) const;
  /**
   * \brief get sample rate indicated in query
   * \return sample rate
   */
  uint get_sample_rate() const;
  /**
   * \brief get AudioQuery volume scale
   * \return volume scale
   */
  double get_volume() const;
  /**
   * \brief convert query to rhubarb phoneme timeline
   * \return Phone timeline
   */
  BoundedTimeline<Phone> to_timeline() const;

private:
  void extract_chunks_();

  static void cons_to_phone_(
    std::string cons,
    std::vector<Timed<Phone>>& phones,
    double start_t,
    double end_t);
  static void vow_to_phone_(
    std::string vow,
    std::vector<Timed<Phone>>& phones,
    double start_t,
    double end_t);

  nlohmann::json json_;
  // vector of chunks - a collection of continous accent phrases
  std::vector<nlohmann::json> chunks_;
};

}  // namespace voicevox
