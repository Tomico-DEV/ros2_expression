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

const nlohmann::json& AudioQuery::get() const
{
    return json_;
}

}