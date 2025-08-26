#pragma once

#include <memory>
#include <filesystem>
#include <vector>

#include "voicevox_core.h"
#include "voicevox_speaker/voicevox_utils.hpp"
#include "voicevox_speaker/synthesizer.hpp"

namespace voicevox
{

/**
 * \brief Voicevox_core c++ wrapper
 */
class Voicevox
{
public:
    /**
     * \brief initialize voicevox related components
     * \param dict_path path to openJtalk dictionary
     * \param ort_path path to onnxruntime library
     * \param model_paths vector of paths to voicemodels to load
     * \param init_options options for synthesizer init
     * 
     */
    Voicevox(
        const std::filesystem::path& dict_path,
        const std::filesystem::path& ort_path,
        std::vector<std::filesystem::path> model_paths,
        InitializeOptions init_options
    );

    /**
     * \param text input text
     * \param style_id which model to use
     */
    AudioQuery make_audio_query(
        const std::string& text,
        uint32_t style_id
    );

    /**
     * \brief synthesize audio from text
     * \param text input text
     * \param style_id which model to use
     * \param interrogative question or not
     * \return WavAudio object containing audio data
     */
    WavAudio tts(
        const std::string& text,
        uint32_t style_id,
        bool interrogative
    );

    /**
     * \brief synthesize audio from AudioQuery
     * \param query audio query
     * \param style_id which model to use
     * \param interrogative question or not
     * \return WavAudio object containing audio data
     */
    WavAudio synthesize(
        const AudioQuery& query,
        uint32_t style_id,
        bool interrogative 
    );

private:
    void ort_load_once_(
        LoadOrtOptions& options
    );

    const Onnxruntime * onnxruntime_;
    std::unique_ptr<Synthesizer> p_synth_;

};

}

 
