#include "voicevox_speaker/speaker_node.hpp"

namespace voicevox
{

VVSpeakerNode::VVSpeakerNode()
: Node("voicevox_speaker"), sound_ { buffer_ }
{
    // filepaths
    std::filesystem::path package_path = ament_index_cpp::get_package_share_directory("voicevox_speaker");
    std::filesystem::path dict_path = package_path / "dict" / "open_jtalk_dic_utf_8-1.11";
    std::filesystem::path ort_path = package_path / "onnx_lib" / voicevox_get_onnxruntime_lib_versioned_filename();
    std::filesystem::path model_path = package_path / "models" / "vvms" / "0.vvm";
        
    std::string text { "こんにちは" };

    p_voicevox_ = std::make_unique<Voicevox>(
        dict_path,
        ort_path,
        std::vector<std::filesystem::path>{ model_path },
        voicevox_make_default_initialize_options()
    );
    
    
    WavAudio result = p_voicevox_->synthesize(text, 3, false);
    
    
    // Playback with SFML
    if (!buffer_.loadFromMemory(result.get_data(), result.get_size()))
    {
        throw std::runtime_error("Coudln't load WAV from memory!");
    }
    
    sound_.play();

}

}