\
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
 * \file synthesizer.cpp
 * \details Synthesizer class implementation
 *
 * See header for details
 *
 * \copyright TomicoDEV 2025
 */

#include "voicevox_speaker/synthesizer.hpp"

namespace voicevox
{

void Synthesizer::load_models(
  std::vector<std::filesystem::path> & model_fpaths)
{
  for (auto & model_file : model_fpaths) {
    VoiceModel model{model_file};
    load_voice_model_(model);
  }
}

/**
 * \brief try load voice model from provided path
 * \param model model to load
 */
void Synthesizer::load_voice_model_(VoiceModel & model) {
  throw_if_err(voicevox_synthesizer_load_voice_model(synth_, model.get()));
}

}  // namespace voicevox
