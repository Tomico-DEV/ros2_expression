#pragma once

#include <atomic> 
#include <thread>
#include <functional>
#include <codecvt>
#include <locale>

#include <SFML/Audio.hpp>

#include "ament_index_cpp/get_package_share_directory.hpp"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"

#include "voicevox_speaker/voicevox.hpp"
#include "voicevox_speaker/synthesis_stream.hpp"
#include "voicevox_speaker/visibility_control.h"

#include "animation/mouthAnimation.h"

#include "speaker_actions/action/speak.hpp"

namespace voicevox
{

using SpeakAction = speaker_actions::action::Speak;
using GoalHandleSpeak = rclcpp_action::ServerGoalHandle<SpeakAction>;

class VVSpeakerNode : public rclcpp::Node
{
public:
    VOICEVOX_SPEAKER_CPP_PUBLIC
    VVSpeakerNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

private:
    void declare_params_();

    /*
    void tts_(const std::string& text);
    */

    void synthesize_(
        std::shared_ptr<AudioQuery> p_query,
        const std::string& text
    );

    void init_action_server_();
    rclcpp_action::GoalResponse handle_speak_goal_(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const SpeakAction::Goal> goal
    );
    rclcpp_action::CancelResponse handle_speak_cancel_(
        const std::shared_ptr<GoalHandleSpeak> goal_handle
    );
    void handle_accepted_(
        const std::shared_ptr<GoalHandleSpeak> goal_handle
    );
    void playback_(
        const std::shared_ptr<GoalHandleSpeak> goal_handle
    );
    void stop_playback_();

    std::filesystem::path dict_path_;
    std::filesystem::path ort_path_;
    std::vector<std::filesystem::path> model_paths_;

    std::shared_ptr<Voicevox> p_voicevox_;
    std::shared_ptr<AudioQuery> p_query_;
    /*
    sf::SoundBuffer buffer_;
    sf::Sound sound_;
    */

    std::jthread playback_thread_;
    std::atomic_bool running_ { false };


    uint32_t speaker_id_ = 3;

    rclcpp_action::Server<SpeakAction>::SharedPtr action_server_;
};

}
