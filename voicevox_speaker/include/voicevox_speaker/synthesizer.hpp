#pragma once

#include <vector>
#include <filesystem>

#include "voicevox_core/voicevox_core.h"
#include "voicevox_speaker/voicevox_utils.hpp"

namespace voicevox
{

class Synthesizer
{
public:
    inline Synthesizer(
        const Onnxruntime * ort,
        OpenJtalkAnalyzer& open_jtalk,
        InitializeOptions options
    )
    {
        throw_if_err(
            voicevox_synthesizer_new(
                ort, open_jtalk.get(), options, &synth_
            )
        );
    }
    inline ~Synthesizer()
    {
        voicevox_synthesizer_delete(synth_);
    }
    inline VoicevoxSynthesizer * get()
    {
        return synth_;
    }

    void load_models(std::vector<std::filesystem::path>& model_fpaths);

private:
    void load_voice_model_(VoiceModel& model);

    VoicevoxSynthesizer * synth_;
};

}
