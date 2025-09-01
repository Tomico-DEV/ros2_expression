#include "voicevox_speaker/voicevox_utils.hpp"

#include "voicevox_speaker/synthesizer.hpp"

namespace voicevox
{

void throw_if_err(VoicevoxResultCode result)
{
    if (result != VOICEVOX_RESULT_OK)
        throw std::runtime_error(voicevox_error_result_to_message(result));
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

}