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
    std::unique_ptr<Voicevox> p_voicevox_;
    sf::SoundBuffer buffer_;
    sf::Sound sound_;
};


}
