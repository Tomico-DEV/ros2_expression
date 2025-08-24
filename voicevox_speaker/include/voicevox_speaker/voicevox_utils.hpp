#pragma once

#include <stdexcept>
#include <filesystem>

#include <iostream>

#include "voicevox_core.h"

namespace voicevox
{

void throw_if_err(VoicevoxResultCode result);

/**
 * \brief c++ wrapper for Voicevox WAV
 */
class WavAudio 
{
public:
    inline WavAudio(size_t wav_size, uint8_t * wav)
    : size_ { wav_size }, data_ { wav }
    {}

    inline ~WavAudio()
    {
        voicevox_wav_free(data_);
    }

    inline void * get_data()
    {
        return static_cast<void *>(data_);
    }
    inline size_t get_size()
    {
        return size_;
    }
private:
    size_t size_;
    uint8_t * data_;
};

typedef VoicevoxInitializeOptions InitializeOptions;
typedef VoicevoxOnnxruntime Onnxruntime;

class LoadOrtOptions
{
public:
    inline LoadOrtOptions(const std::filesystem::path& ort_file)
    : option_ { voicevox_make_default_load_onnxruntime_options() }
    {
        option_.filename = ort_file.c_str();
    }
    inline VoicevoxLoadOnnxruntimeOptions get()
    {
        return option_;
    }
private:
    VoicevoxLoadOnnxruntimeOptions option_;
};

class OpenJtalkAnalyzer
{
public:
    inline OpenJtalkAnalyzer(const std::filesystem::path& dict_file)
    : source_fpath_ { dict_file.string() }
    {
        throw_if_err(
            voicevox_open_jtalk_rc_new(dict_file.c_str(), &open_jtalk_)
        );
    }
    inline OpenJtalkAnalyzer(const OpenJtalkAnalyzer& obj)
    : source_fpath_ { obj.source_fpath_ }
    {
        throw_if_err(
            voicevox_open_jtalk_rc_new(source_fpath_.c_str(), &open_jtalk_)
        );
    }
    inline ~OpenJtalkAnalyzer()
    {
        voicevox_open_jtalk_rc_delete(open_jtalk_);
    }
    inline OpenJtalkRc * get()
    {
        return open_jtalk_;
    }
private:
    OpenJtalkRc * open_jtalk_;
    std::string source_fpath_;
};

class VoiceModel
{
public:
    inline VoiceModel(std::filesystem::path model_fpath)
    {
        throw_if_err(
            voicevox_voice_model_file_open(model_fpath.c_str(), &model_)
        );
    }
    inline ~VoiceModel()
    {
        voicevox_voice_model_file_delete(model_);
    }
    inline VoicevoxVoiceModelFile * get()
    {
        return model_;
    }

private:
    VoicevoxVoiceModelFile * model_;
};

}