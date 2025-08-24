#include "voicevox_speaker/speaker_node.hpp"

namespace voicevox
{

VVSpeakerNode::VVSpeakerNode()
: Node("voicevox_speaker"), sound_ { buffer_ }
{
    std::string text { "こんにちは" };

    p_voicevox_ = std::make_unique<Voicevox>(
        dict_path_,
        ort_path_,
        model_paths_,
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

void VVSpeakerNode::declare_params()
{
    typedef rcl_interfaces::msg::ParameterDescriptor ParamDesc;
    
    auto make_desc = [&](const std::string &desc_text) {
        ParamDesc desc;
        desc.description = desc_text;
        return desc;
    };

    // default paths
    std::filesystem::path package_path = ament_index_cpp::get_package_share_directory("voicevox_speaker");
    std::filesystem::path dict_path = package_path / "dict" / "open_jtalk_dic_utf_8-1.11";
    std::filesystem::path ort_path = package_path / "onnx_lib" / voicevox_get_onnxruntime_lib_versioned_filename();
    std::filesystem::path model_path = package_path / "models" / "vvms" / "0.vvm";
        
    dict_path_ = declare_parameter<std::string>(
        "voicevox.dictionary_path",
        dict_path.string(),
        make_desc("Path to OpenJtalk dictionary")
    );
    ort_path_ = declare_parameter<std::string>(
        "voicevox.onnxruntime_lib_path",
        dict_path.string(),
        make_desc("Path to onnxruntime library")
    );
    auto model_pathstrings = declare_parameter<std::vector<std::string>>(
        "voicevox.model_path_",
        { model_path.string() },
        make_desc("Paths to voicemodel files to load")
    );
    // convert strings to filepaths
    model_paths_.reserve(model_pathstrings.size());
    std::transform(
        model_pathstrings.begin(),
        model_pathstrings.end(),
        std::back_inserter(model_paths_),
        [](const std::string& s) { return std::filesystem::path { s }; }
    );

    
}

}