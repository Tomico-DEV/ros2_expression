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

#include <memory>
#include <filesystem>
#include <string>
#include <vector>

// Voicevox
#include "voicevox_core/voicevox_core.h"
#include "voicevox_speaker/voicevox_utils.hpp"
#include "voicevox_speaker/synthesizer.hpp"

namespace voicevox
{

/**
 * \brief Voicevox_core c++ wrapper
 */
class Voicevox
{
public:
  /**
   * \brief initialize voicevox related components
   * \param dict_path path to openJtalk dictionary
   * \param ort_path path to onnxruntime library
   * \param model_paths vector of paths to voicemodels to load
   * \param init_options options for synthesizer init
   * 
   */
  Voicevox(
    const std::filesystem::path & dict_path,
    const std::filesystem::path & ort_path,
    std::vector<std::filesystem::path> model_paths,
    InitializeOptions init_options);

  /**
   * \param text input text
   * \param style_id which model to use
   */
  AudioQuery make_audio_query(
    const std::string & text,
    uint32_t style_id);

  /**
   * \brief synthesize audio from text
   * \param text input text
   * \param style_id which model to use
   * \param interrogative question or not
   * \return WavAudio object containing audio data
   */
  WavAudio tts(
    const std::string & text,
    uint32_t style_id,
    bool interrogative);

  /**
   * \brief synthesize audio from AudioQuery
   * \param query audio query
   * \param style_id which model to use
   * \param interrogative interrogative tone or not
   * \return WavAudio object containing audio data
   */
  WavAudio synthesize(
    const AudioQuery & query,
    uint32_t style_id,
    bool interrogative);

  /**
   * \brief synthesize audio from AudioQuery at ith chunk
   * \param query audio query
   * \param i which chunk to synthesize
   * \param style_id which model to use
   * \return WavAudio object containing audio data
   */
  WavAudio synthesize_chunk(
    const AudioQuery & query,
    uint i,
    uint32_t style_id);

private:
  void ort_load_once_(
    const LoadOrtOptions & options);

  const Onnxruntime * onnxruntime_;
  std::unique_ptr<Synthesizer> p_synth_;
};

}  // namespace voicevox
