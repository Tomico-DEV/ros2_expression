#include "voicevox_speaker/synthesizer.hpp"

namespace voicevox
{

void Synthesizer::load_models(std::vector<std::filesystem::path>& model_fpaths)
{
    for (auto& model_file : model_fpaths)
    {
        VoiceModel model { model_file };
        load_voice_model_(model);
    }
}

void Synthesizer::load_voice_model_(VoiceModel& model)
{
    throw_if_err(
        voicevox_synthesizer_load_voice_model(synth_, model.get())
    );
}

}