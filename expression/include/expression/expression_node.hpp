
#pragma once

#include <atomic>
#include <string>
#include <memory>
#include <unordered_map>
#include <thread>
#include <vector>

// ROS2
#include "speaker_actions/action/speak.hpp"
#include "speaker_actions/action/tts.hpp"

#include "face_msgs/msg/param1_d.hpp"
#include "face_msgs/msg/param2_d.hpp"

#include "ament_index_cpp/get_package_share_directory.hpp"

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"

// BT
#include "behaviortree_cpp/bt_factory.h"

// Tweeny
#include "tweeny.h"

#include "expression/eye_publishers.hpp"
#include "expression/mouth_publishers.hpp"
#include "expression/neck_publishers.hpp"
#include "expression/utils.hpp"
#include "expression/channel.hpp"
#include "expression/breath_animation.hpp"

#include "expression/visibility_control.h"


namespace expression
{

using SpeakAction = speaker_actions::action::Speak;
using GoalHandleSpeak = rclcpp_action::ClientGoalHandle<SpeakAction>;
using TtsAction = speaker_actions::action::Tts;
using GoalHandleTts = rclcpp_action::ClientGoalHandle<TtsAction>;

class ExpressionNode : public rclcpp::Node
{
public:
  EXPRESSION_SERVER_CPP_PUBLIC
  explicit ExpressionNode(
    const rclcpp::NodeOptions &options = rclcpp::NodeOptions());
  ~ExpressionNode();



private:
  void declare_params_();

  void create_publishers_();
  void create_animators_();

  void start_channels_();
  void stop_channels_();

  void start_animation_();
  void stop_animation_();
  void update_animation_();

  inline bool has_speak_task_()
  {
    return false;
  }

  rclcpp_action::Client<SpeakAction>::SharedPtr p_speak_client_;
  rclcpp_action::Server<TtsAction>::SharedPtr p_tts_server_;


  // face publishers
  EyePublishers::SharedPtr p_left_eye_pubs_;
  EyePublishers::SharedPtr p_right_eye_pubs_;
  MouthPublishers::SharedPtr p_mouth_pubs_;
  NeckPublishers::SharedPtr p_neck_pubs_;
  Channel1D::SharedPtr p_diaphragm;

  // animators
  BreathAnimator::UniquePtr p_breath_animator;

  std::atomic_bool running_{false};
  std::jthread animator_thread_;


  std::chrono::milliseconds animate_rate_{10};
};

}  // namespace expression
