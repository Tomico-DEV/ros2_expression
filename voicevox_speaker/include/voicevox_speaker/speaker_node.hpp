#pragma once

#include <SFML/Audio.hpp>

#include "rclcpp/rclcpp.hpp"
#include "voicevox_speaker/voicevox.hpp"
#include "ament_index_cpp/get_package_share_directory.hpp"

namespace voicevox
{

class VVSpeakerNode : public rclcpp::Node
{
public:
    VVSpeakerNode();

private:
    void declare_params();

    std::filesystem::path dict_path_;
    std::filesystem::path ort_path_;
    std::vector<std::filesystem::path> model_paths_;

    std::unique_ptr<Voicevox> p_voicevox_;
    sf::SoundBuffer buffer_;
    sf::Sound sound_;
};


}
