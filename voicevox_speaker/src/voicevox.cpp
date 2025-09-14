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
 * \file voicevox.cpp
 * \details Voicevox class implementation
 *
 * See header for details
 */

#include "voicevox_speaker/voicevox.hpp"


namespace voicevox
{

Voicevox::Voicevox(
  const std::filesystem::path & dict_path,
  const std::filesystem::path & ort_path,
  std::vector<std::filesystem::path> model_paths,
  InitializeOptions init_options)
{
  // load ort
  LoadOrtOptions load_ort_options{ort_path};
  ort_load_once_(load_ort_options);

  // load OpenJtalk
  OpenJtalkAnalyzer open_jtalk{dict_path};

  // create sythensizer
  p_synth_ = std::make_unique<Synthesizer>(
    onnxruntime_,
    open_jtalk,
    init_options);
  p_synth_->load_models(model_paths);
}

auto Voicevox::make_audio_query(
  const std::string & text,
  uint32_t style_id
) -> AudioQuery
{
  return AudioQuery{
    *p_synth_,
    text,
    style_id
  };
}

auto Voicevox::tts(
  const std::string & text,
  uint32_t style_id,
  bool interrogative
) -> WavAudio
{
  size_t output_wav_size = 0;
  uint8_t * output_wav = nullptr;

  VoicevoxTtsOptions options;
  options.enable_interrogative_upspeak = interrogative;

  throw_if_err(
    voicevox_synthesizer_tts(
      p_synth_->get(), text.c_str(), style_id, options,
      &output_wav_size, &output_wav));

  return WavAudio{output_wav_size, output_wav};
}

auto Voicevox::synthesize(
  const AudioQuery & query,
  uint32_t style_id,
  bool interrogative
)-> WavAudio
{
  size_t output_wav_size = 0;
  uint8_t * output_wav = nullptr;

  VoicevoxSynthesisOptions options;
  options.enable_interrogative_upspeak = interrogative;

  throw_if_err(
    voicevox_synthesizer_synthesis(
      p_synth_->get(),
      query.get().dump().c_str(),
      style_id, options,
      &output_wav_size, &output_wav));

  return WavAudio{output_wav_size, output_wav};
}

auto Voicevox::synthesize_chunk(
  const AudioQuery & query,
  uint i,
  uint32_t style_id
)-> WavAudio
{
  size_t output_wav_size = 0;
  uint8_t * output_wav = nullptr;

  VoicevoxSynthesisOptions options = voicevox_make_default_synthesis_options();
  // options.enable_interrogative_upspeak = interrogative;

  throw_if_err(
    voicevox_synthesizer_synthesis(
      p_synth_->get(),
      query.get_chunk(i).dump().c_str(),
      style_id, options,
      &output_wav_size, &output_wav));

  return WavAudio{output_wav_size, output_wav};
}

void Voicevox::ort_load_once_(
  const LoadOrtOptions & option
)
{
  VoicevoxResultCode result =
    voicevox_onnxruntime_load_once(option.get(), &onnxruntime_);

  if (result != VOICEVOX_RESULT_OK) {
    throw std::runtime_error(voicevox_error_result_to_message(result));
  }
}

}  // namespace voicevox
