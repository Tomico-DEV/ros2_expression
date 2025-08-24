#include "voicevox_speaker/voicevox.hpp"

#include <iostream>

namespace voicevox
{

Voicevox::Voicevox(
    const std::filesystem::path& dict_path,
    const std::filesystem::path& ort_path,
    std::vector<std::filesystem::path> model_paths,
    InitializeOptions init_options
)
{
    // load ort
    LoadOrtOptions load_ort_options { ort_path };
    ort_load_once_(load_ort_options);

    // load OpenJtalk
    OpenJtalkAnalyzer open_jtalk { dict_path };

    // create sythensizer
    p_synth_ = std::make_unique<Synthesizer>(
        onnxruntime_,
        open_jtalk,
        init_options
    );
    p_synth_->load_models(model_paths);
}

auto Voicevox::synthesize(
    const std::string& text,
    uint8_t style_id,
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
            &output_wav_size, &output_wav
        )
    );

    return WavAudio { output_wav_size, output_wav };
}

void Voicevox::ort_load_once_(
    LoadOrtOptions& option
)
{
    VoicevoxResultCode result = 
        voicevox_onnxruntime_load_once(option.get(), &onnxruntime_);

    if (result != VOICEVOX_RESULT_OK)
        throw std::runtime_error(voicevox_error_result_to_message(result));
}

}