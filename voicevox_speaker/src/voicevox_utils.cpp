#include "voicevox_speaker/voicevox_utils.hpp"

#include "voicevox_speaker/synthesizer.hpp"

namespace voicevox
{

void throw_if_err(VoicevoxResultCode result)
{
    if (result != VOICEVOX_RESULT_OK)
        throw std::runtime_error(voicevox_error_result_to_message(result));
}

std::u32string to_utf32(const std::string & input) 
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.from_bytes(input);
}

AudioQuery::AudioQuery(
    Synthesizer& synth,
    std::string text,
    uint32_t style_id
)
{
    char * raw_string;
    voicevox_synthesizer_create_audio_query(
        synth.get(),
        text.c_str(),
        style_id,
        &raw_string
    );

    json_ = nlohmann::json::parse(raw_string);

    voicevox_json_free(raw_string);

    std::u32string jp_string = to_utf32(text);
    bool question = ((jp_string.back() == '?') | (jp_string.back() == U'？'));
    json_["accent_phrases"].back()["is_interrogative"] = question;
}

uint AudioQuery::get_query_length() const
{
    return json_["accent_phrases"].size();
}

nlohmann::json AudioQuery::get_chunk(uint i) const
{
    if (!json_.contains("accent_phrases") || !json_["accent_phrases"].is_array()) {
        throw std::runtime_error("JSON does not contain accent_phrases array!");
    }
    if (i >= json_["accent_phrases"].size()) {
        throw std::out_of_range("Accent phrase index out of range");
    }

    nlohmann::json chunk = json_;
    chunk["accent_phrases"] = nlohmann::json::array({ json_["accent_phrases"][i] });

    return chunk;
}

const nlohmann::json& AudioQuery::get() const
{
    return json_;
}

centiseconds sec2centi(double secs)
{
    return std::chrono::round<centiseconds>(
        std::chrono::duration<double>(secs)
    );

}

auto AudioQuery::to_timeline() const -> BoundedTimeline<Phone>
{
    std::vector<Timed<Phone>> phones_;
    double cumul_time = 0.0;
    for (const auto& phrase : json_["accent_phrases"])
    {
        for (const auto& mora : phrase["moras"])
        {
            // consonants
            if (!mora["consonant"].is_null())
            {
                double start_t = cumul_time;
                double end_t = cumul_time + mora["consonant_length"].get<double>();
                Timed<Phone> t_phone {
                    sec2centi(start_t),
                    sec2centi(end_t),
                    cons_to_phone_(mora["consonant"].get<std::string>())
                };
                phones_.push_back(t_phone);
                cumul_time = end_t;
            }
            // vowels
            if (!mora["vowel"].is_null())
            {
                double start_t = cumul_time;
                double end_t = cumul_time + mora["vowel_length"].get<double>();
                Timed<Phone> t_phone {
                    sec2centi(start_t),
                    sec2centi(end_t),
                    vow_to_phone_(mora["vowel"].get<std::string>())
                };
                phones_.push_back(t_phone);
                cumul_time = end_t;
            }
        }
        // pause
        const auto& pause = phrase["pause_mora"];
        if (!pause.is_null())
        {
            // sanity check
            if (pause["vowel"].get<std::string>() != "pau")
                throw std::runtime_error(
                    std::format("Unknown pause vowel: {}", pause["vowel"].get<std::string>())
                );
            cumul_time += pause["vowel_length"].get<double>();
        }
    }

    // create timeline
    BoundedTimeline<Phone> timeline { TimeRange{sec2centi(0), sec2centi(cumul_time)} };
    for (const auto& t_phone : phones_)
    {
        timeline.set(t_phone);
    }

    return timeline;
}

auto AudioQuery::cons_to_phone_(std::string cons) -> Phone
{
    static const std::unordered_map<std::string, Phone> table = {
        // K/G
        {"k",  Phone::K},   // カ行
        {"g",  Phone::G},   // ガ行

        // S/Z
        {"s",  Phone::S},   // サ行
        {"z",  Phone::Z},   // ザ行
        {"sh", Phone::SH},  // シ
        {"j",  Phone::JH},  // ジ

        // T/D
        {"t",  Phone::T},   // タ行
        {"d",  Phone::D},   // ダ行
        {"ch", Phone::CH},  // チ
        {"ts", Phone::S},   // ツ (special)


        // H/F/B/P
        {"h",  Phone::HH},  // ハ行
        {"f",  Phone::F},   // フ
        {"b",  Phone::B},   // バ行
        {"p",  Phone::P},   // パ行

        // N
        {"n",  Phone::N},   // ナ行

        // M
        {"m",  Phone::M},   // マ行

        // Y
        {"y",  Phone::Y},   // ヤ行

        // R
        {"r",  Phone::R},   // ラ行

        // W
        {"w",  Phone::W},   // ワ行
    };

    auto it = table.find(cons);
    if (it != table.end())
        return it->second;

    throw std::runtime_error(std::format("Unknown consonant: {}", cons));
}

auto AudioQuery::vow_to_phone_(std::string vow) -> Phone
{
    static const std::unordered_map<std::string, Phone> table = {
        // あいうえお
        {"a", Phone::AA},
        {"i", Phone::IY},
        {"u", Phone::UW},
        {"e", Phone::EY},
        {"o", Phone::OW},
        // ん
        {"N", Phone::N},
        // す (u) after s
        {"U", Phone::S}
    };

    auto it = table.find(vow);
    if (it != table.end())
        return it->second;

    throw std::runtime_error(std::format("Unknown vowel: {}", vow));
}

}