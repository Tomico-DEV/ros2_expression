
#pragma once

#include <atomic>
#include <format>
#include <string>
#include <memory>
#include <map>
#include <thread>
#include <vector>

// ROS2
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "rclcpp_components/register_node_macro.hpp"

#include "ament_index_cpp/get_package_share_directory.hpp"

#include "speaker_actions/action/speak.hpp"
#include "speaker_actions/action/tts.hpp"

#include "lifecycle_msgs/msg/state.hpp"

#include "face_msgs/msg/param1_d.hpp"
#include "face_msgs/msg/param2_d.hpp"

// Tweeny
#include "tweeny/tweeny.h"

#include "expression/eye_channels.hpp"
#include "expression/mouth_channels.hpp"
#include "expression/neck_channels.hpp"
#include "expression/utils.hpp"
#include "expression/animator.hpp"

#include "expression/visibility_control.h"

// plugins
#include <pluginlib/class_loader.hpp>

namespace expression
{

using SpeakAction = speaker_actions::action::Speak;
using GoalHandleSpeak = rclcpp_action::ClientGoalHandle<SpeakAction>;
using TtsAction = speaker_actions::action::Tts;
using GoalHandleTts = rclcpp_action::ClientGoalHandle<TtsAction>;

class ExpressionNode : public rclcpp_lifecycle::LifecycleNode
{
public:
  EXPRESSION_SERVER_CPP_PUBLIC
  explicit ExpressionNode(
    const rclcpp::NodeOptions &options = rclcpp::NodeOptions());
  ~ExpressionNode() override;

protected:
  // lifecycle functions
  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override;
  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override;

private:
  void declare_params_();

  void create_channels_();
  void create_animators_();

  void start_channels_();
  void stop_channels_();

  void start_animation_();
  void stop_animation_();
  void update_animation_();

  void cleanup_();

  inline bool has_speak_task_()
  {
    return false;
  }

  rclcpp_action::Client<SpeakAction>::SharedPtr p_speak_client_;
  rclcpp_action::Server<TtsAction>::SharedPtr p_tts_server_;


  // face publishers
  EyeChannels::SharedPtr p_left_eye_chans_;
  EyeChannels::SharedPtr p_right_eye_chans_;
  MouthChannels::SharedPtr p_mouth_chans_;
  NeckChannels::SharedPtr p_neck_chans_;

  // channel map
  std::shared_ptr<ChanMap> p_chan_map_;

  // animators
  std::unique_ptr<pluginlib::ClassLoader<Animator>> animator_loader_;
  std::vector<std::string> animator_plugins_str_;
  std::vector<Animator::SharedPtr> animators_;

  std::atomic_bool running_{false};
  std::jthread animator_thread_;

  std::chrono::milliseconds animate_rate_{10};
};

}  // namespace expression
