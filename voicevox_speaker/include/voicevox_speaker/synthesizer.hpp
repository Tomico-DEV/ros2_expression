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
 * \file synthesizer.hpp
 * \brief Wrapper for Voicevox's synthesizer class
 */

#pragma once

#include <vector>
#include <filesystem>

// Voicevox
#include "voicevox_core/voicevox_core.h"
#include "voicevox_speaker/voicevox_utils.hpp"

namespace voicevox
{

class Synthesizer
{
public:
  /**
   * \param ort Onnxruntime object
   * \param open_jtalk OpenJTalk Dictionary object
   * \param options Options to pass to voicevox on initialization
   */
  inline Synthesizer(
    const Onnxruntime * ort,
    const OpenJtalkAnalyzer & open_jtalk,
    InitializeOptions options)
  {
    throw_if_err(
      voicevox_synthesizer_new(
          ort, open_jtalk.get(), options, &synth_));
  }
  inline ~Synthesizer()
  {
    voicevox_synthesizer_delete(synth_);
  }
  /**
   * \brief get underlying VoicevoxSynthesizer
   */
  inline VoicevoxSynthesizer * get() const
  {
    return synth_;
  }
  /**
   * \brief load models from a vector of filepaths
   */
  void load_models(std::vector<std::filesystem::path> & model_fpaths);

private:
  void load_voice_model_(VoiceModel & model);

  VoicevoxSynthesizer * synth_;
};

}  // namespace voicevox
