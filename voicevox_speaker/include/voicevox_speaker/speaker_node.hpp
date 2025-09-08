// Copyright 2025 TomicoDEV
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 *                    -
 *   /\                 \       _________________
 *  //\\                 \     /                 \
 * //  \\          <<<    |   |  ROS2 EXPRESSION  |
 *             <<<<        |   \ ________________/
 *          <<             |   |/ 
 *             <<<<        |
 *                 <<<    |
 *                       /
 *                      /
 *                    -
 * \author TomicoDEV
 * \file speaker_node.hpp
 * \brief Expression Speaker Node
 * 
 * TTS Node utilizing Voicevox. TTS is done through actions
 * 
 * Parameters:
 *  voicevox. ...
 * 
 * Action server:
 * 
*/

#pragma once

#include <atomic>
#include <algorithm>
#include <codecvt>
#include <functional>
#include <locale>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

// SFML
#include <SFML/Audio.hpp>

// ROS2
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"

#include "ament_index_cpp/get_package_share_directory.hpp"

#include "speaker_actions/action/speak.hpp"

// Voicevox
#include "voicevox_speaker/synthesis_stream.hpp"
#include "voicevox_speaker/visibility_control.h"
#include "voicevox_speaker/voicevox.hpp"

// Rhubarb
#include "animation/mouthAnimation.h"


namespace voicevox
{

using SpeakAction = speaker_actions::action::Speak;
using GoalHandleSpeak = rclcpp_action::ServerGoalHandle<SpeakAction>;

class VVSpeakerNode : public rclcpp::Node
{
public:
  VOICEVOX_SPEAKER_CPP_PUBLIC
  explicit VVSpeakerNode(
    const rclcpp::NodeOptions &options = rclcpp::NodeOptions());

private:
  // Node functions

  void declare_params_();

  // Action server functions

  void init_action_server_();

  auto handle_speak_goal_(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const SpeakAction::Goal> goal)
  -> rclcpp_action::GoalResponse;

  auto handle_speak_cancel_(const std::shared_ptr<GoalHandleSpeak> goal_handle)
  -> rclcpp_action::CancelResponse;

  void handle_accepted_(const std::shared_ptr<GoalHandleSpeak> goal_handle);

  // playback and synthesis

  void playback_(
    const std::shared_ptr<GoalHandleSpeak> goal_handle,
    double update_rate);
  void stop_playback_();

  // Voicevox runtime

  std::filesystem::path dict_path_;
  std::filesystem::path ort_path_;
  std::vector<std::filesystem::path> model_paths_;

  // Voicevox

  std::shared_ptr<Voicevox> p_voicevox_;
  std::shared_ptr<AudioQuery> p_query_;
  uint32_t speaker_id_ = 3;

  // Rhubarb
  ShapeSet mouth_shapes_;

  // Node variables

  rclcpp_action::Server<SpeakAction>::SharedPtr action_server_;

  double update_rate_ = 100;  // rate to send feedback
  std::jthread playback_thread_;
  std::atomic_bool running_{false};
};

}  // namespace voicevox

