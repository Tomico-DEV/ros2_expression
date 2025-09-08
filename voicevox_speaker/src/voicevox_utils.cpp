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
 *             <<<<        |   \ ________________
 *          <<             |   |/
 *             <<<<        |
 *                 <<<    |
 *                       /
 *                      /
 *                    -
 * \author TomicoDEV
 * \file voicevox_utils.cpp
 * \details Implementation of voicevox utility functions and classes
 *
 * See header for details
 */

#include "voicevox_speaker/voicevox_utils.hpp"

#include "voicevox_speaker/synthesizer.hpp"

namespace voicevox
{

/**
 * \brief convenience function to convert from double to 
 *        centiseconds
 */
centiseconds sec2centi(double secs)
{
  return std::chrono::round<centiseconds>(
    std::chrono::duration<double>(secs));
}

void throw_if_err(VoicevoxResultCode result)
{
  if (result != VOICEVOX_RESULT_OK) {
    throw std::runtime_error(voicevox_error_result_to_message(result));
  }
}

std::u32string to_utf32(const std::string & input)
{
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  return conv.from_bytes(input);
}

AudioQuery::AudioQuery(
  const Synthesizer & synth, std::string text, uint32_t style_id)
{
  char * raw_string;
  voicevox_synthesizer_create_audio_query(
    synth.get(), text.c_str(), style_id, &raw_string);

  json_ = nlohmann::json::parse(raw_string);

  voicevox_json_free(raw_string);  // raw string has been copied, can free

  extract_chunks_();

  // json modification
  // we don't need any padding
  json_["prePhonemeLength"] = 0.0;
  json_["postPhonemeLength"] = 0.0;
}

uint AudioQuery::get_query_length() const
{
  return chunks_.size();
}

uint AudioQuery::get_sample_rate() const
{
  return json_["outputSamplingRate"].get<unsigned int>();
}

auto AudioQuery::get_chunk(uint i) const -> nlohmann::json
{
  // safety check
  if (i >= chunks_.size()) {
    throw std::out_of_range("Chunk index out of range!");
  }

  // copy json and substitute accent phrases
  nlohmann::json json_copy = json_;
  json_copy["accent_phrases"] = chunks_[i];

  return json_copy;
}

auto AudioQuery::get() const -> const nlohmann::json &
{
  return json_;
}

auto AudioQuery::get_chunk_dur(uint i) const -> double
{
  if (i >= chunks_.size()) {
    throw std::out_of_range("Chunk index out of range!");
  }

  double cumul_time = 0.0;
  const auto & chunk = chunks_[i];

  for (const auto & phrase : chunk) {
    for (const auto & mora : phrase["moras"]) {
      // consonants
      if (!mora["consonant"].is_null()) {
        cumul_time += mora["consonant_length"].get<double>();
      }
      // vowels
      if (!mora["vowel"].is_null()) {
        cumul_time += mora["vowel_length"].get<double>();
      }
    }
    // pause
    const auto & pause = phrase["pause_mora"];
    if (!pause.is_null()) {
      // sanity check
      if (pause["vowel"].get<std::string>() != "pau")
        throw std::runtime_error(
          std::format(
            "Unknown pause vowel: {}",
            pause["vowel"].get<std::string>()));
      cumul_time += pause["vowel_length"].get<double>();
    }
  }

  return cumul_time;
}

double AudioQuery::get_volume() const
{
  return json_["volumeScale"].get<double>();
}

auto AudioQuery::to_timeline() const -> BoundedTimeline<Phone>
{
  std::vector<Timed<Phone>> phones_;
  double cumul_time = 0.0;

  for (const auto & phrase : json_["accent_phrases"]) {
    for (const auto & mora : phrase["moras"]) {
      // consonants
      if (!mora["consonant"].is_null()) {
        double start_t = cumul_time;
        double end_t = cumul_time + mora["consonant_length"].get<double>();

        cons_to_phone_(
          mora["consonant"].get<std::string>(),
          phones_,
          start_t, end_t);

        cumul_time = end_t;
      }

      // vowels
      if (!mora["vowel"].is_null()) {
        double start_t = cumul_time;
        double end_t = cumul_time + mora["vowel_length"].get<double>();

        vow_to_phone_(
          mora["vowel"].get<std::string>(),
          phones_,
          start_t, end_t);

        cumul_time = end_t;
      }
    }
    // pause
    const auto & pause = phrase["pause_mora"];
    if (!pause.is_null()) {
      // sanity check
      if (pause["vowel"].get<std::string>() != "pau")
        throw std::runtime_error(std::format(
            "Unknown pause vowel: {}", pause["vowel"].get<std::string>()));
      cumul_time += pause["vowel_length"].get<double>();
    }
  }

  // create timeline
  BoundedTimeline<Phone> timeline{TimeRange{sec2centi(0), sec2centi(cumul_time)}};
  // finally add phones to timeline
  for (const auto & t_phone : phones_) {
    timeline.set(t_phone);
  }

  return timeline;
}

/**
 * \brief populate chunks_ with.. chunks from the input query
 * 
 * A 'chunk' is a collection of continuous (no pause mora) phrases
 */
void AudioQuery::extract_chunks_()
{
  nlohmann::json chunk = nlohmann::json::array();

  // keep adding chunks till we hit one with a pause mora
  for (auto & phrase : json_["accent_phrases"]) {
    chunk.push_back(phrase);
    if (!phrase["pause_mora"].is_null()) {
      // interrogative sentence check
      std::u32string pause_str =
          to_utf32(phrase["pause_mora"]["text"].get<std::string>());
      bool question = ((pause_str.back() == '?') | (pause_str.back() == U'？'));
      phrase["is_interrogative"] = question;
      chunks_.push_back(chunk);  // add chunk
      chunk = nlohmann::json::array();  // reset chunk
    }
  }

  // just in case
  if (!chunk.empty()) {
    chunks_.push_back(chunk);
  }
}

/**
 * \brief convert consonant to phoneme
 * \param [in]  cons consonant to convert
 * \param [out] phones phone timeline to append converted phoneme to
 * \param start_t start time of the consonant
 * \param end_t end time of consonant
 * 
 * Throws a std::runtime_error if an unknown constant is encountered
 */
void AudioQuery::cons_to_phone_(
  std::string cons,
  std::vector<Timed<Phone>> & phones,
  double start_t, double end_t)
{
  // regular consonants (primary: 1-1 conversion possible)
  static const std::unordered_map<std::string, Phone> table = {
    // K/G
    {"k", Phone::K},    // カ行
    {"g", Phone::G},    // ガ行

    // S/Z
    {"s", Phone::S},    // サ行
    {"z", Phone::Z},    // ザ行
    {"sh", Phone::SH},  // シ
    {"j", Phone::JH},   // ジ

    // T/D
    {"t", Phone::T},    // タ行
    {"d", Phone::D},    // ダ行
    {"ch", Phone::CH},  // チ
    {"ts", Phone::S},   // ツ (special)

    // H/F/B/P
    {"h", Phone::HH},   // ハ行
    {"f", Phone::F},    // フ
    {"b", Phone::B},    // バ行
    {"p", Phone::P},    // パ行

    // N
    {"n", Phone::N},    // ナ行

    // M
    {"m", Phone::M},    // マ行

    // Y
    {"y", Phone::Y},    // ヤ行

    // R
    {"r", Phone::R},    // ラ行

    // W
    {"w", Phone::W}     // ワ行
  };

  // speicial consonants (composite; made up of several phonemes)
  static const std::unordered_map<
    std::string, std::vector<std::pair<Phone, double>>
  > special = {
    // { string, {phonemes, duration ratios}}

    // Xょ
    {"ky", {{Phone::K, 0.5}, {Phone::Y, 0.5}}},   // キャ, キュ, キョ
    {"gy", {{Phone::G, 0.5}, {Phone::Y, 0.5}}},   // ギャ...
    {"ny", {{Phone::N, 0.5}, {Phone::Y, 0.5}}},   // ニャ...
    {"hy", {{Phone::HH, 0.5}, {Phone::Y, 0.5}}},  // ヒャ...
    {"by", {{Phone::B, 0.5}, {Phone::Y, 0.5}}},   // ビャ...
    {"py", {{Phone::P, 0.5}, {Phone::Y, 0.5}}},   // ピャ...
    {"my", {{Phone::M, 0.5}, {Phone::Y, 0.5}}},   // ミャ...
    {"ry", {{Phone::R, 0.5}, {Phone::Y, 0.5}}},   // リャ...

    {"ts", {{Phone::T, 0.5}, {Phone::S, 0.5}}}    // ツァ, ツィ, etc.
  };

  // special/composite check
  auto special_it = special.find(cons);
  if (special_it != special.end()) {
    double duration = end_t - start_t;
    double head = start_t;
    const auto & phone_dur_pairs = special_it->second;
    for (const auto & pair : phone_dur_pairs) {
      double local_end_t = head + duration * pair.second;
      Timed<Phone> t_phone{sec2centi(head), sec2centi(local_end_t), pair.first};
      phones.push_back(t_phone);
      head = local_end_t;
    }
    return;
  }

  // normal check
  auto it = table.find(cons);
  if (it != table.end()) {
    Timed<Phone> t_phone{sec2centi(start_t), sec2centi(end_t), it->second};
    phones.push_back(t_phone);
    return;
  }

  // no hits
  throw std::runtime_error(std::format("Unknown consonant: {}", cons));
}

void AudioQuery::vow_to_phone_(
  std::string vow,
  std::vector<Timed<Phone>> & phones,
  double start_t, double end_t)
{
  static const std::unordered_map<std::string, Phone> table = {
    // あいうえお
    {"a", Phone::AA},
    {"i", Phone::IY},
    {"u", Phone::UW},
    {"e", Phone::EY},
    {"o", Phone::OW},
    // cut-off vowels
    {"A", Phone::AA},
    {"I", Phone::IY},
    {"U", Phone::UW},
    {"E", Phone::EY},
    {"O", Phone::OW},

    // ん
    {"N", Phone::N}
  };

  auto it = table.find(vow);
  if (it != table.end()) {
    Timed<Phone> t_phone{sec2centi(start_t), sec2centi(end_t), it->second};
    phones.push_back(t_phone);
    return;
  }
  // っ
  if (vow == "cl") {
    return;        // add nothing
  }

  throw std::runtime_error(std::format("Unknown vowel: {}", vow));
}

}  // namespace voicevox
