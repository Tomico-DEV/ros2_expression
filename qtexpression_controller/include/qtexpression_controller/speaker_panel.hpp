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
 * \file speaker_panel.hpp
 * \brief SpeakerPanel - an RVIZ2 panel for testing voicevox_speaker
 *
 * See header for details
 */

#pragma once

// QT
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

// std
#include <memory>

// RVIZ2
#include <rviz_common/panel.hpp>
#include <rviz_common/display_context.hpp>
#include <rviz_common/ros_integration/ros_node_abstraction_iface.hpp>

// ROS2
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "speaker_actions/action/speak.hpp"

namespace qtexpression_controller
{

using SpeakAction = speaker_actions::action::Speak;
using GoalHandleSpeak = rclcpp_action::ClientGoalHandle<SpeakAction>;

class SpeakerPanel : public rviz_common::Panel
{
  Q_OBJECT

public:
  explicit SpeakerPanel(QWidget * parent = 0);
  ~SpeakerPanel() override;

  void onInitialize() override;

protected:
  // action client
  std::shared_ptr<rviz_common::ros_integration::RosNodeAbstractionIface> node_ptr_;
  rclcpp_action::Client<SpeakAction>::SharedPtr client_ptr_;
  rclcpp_action::ClientGoalHandle<SpeakAction>::SharedPtr active_goal_;

  // gui
  QLineEdit * input_text_;
  QLabel * viseme_;
  QLabel * result_;
  QPushButton * send_button_;
  QPushButton * cancel_button_;

private Q_SLOTS:
  void send_speak_goal_();
  void cancel_speak_goal_();

private:
  void create_layout_();
  void set_result_color_(Qt::GlobalColor color);
  void response_callback_(
    const GoalHandleSpeak::SharedPtr & goal_handle);
  void feedback_callback_(
    GoalHandleSpeak::SharedPtr,
    const std::shared_ptr<const SpeakAction::Feedback> feedback);
  void result_callback_(
    const GoalHandleSpeak::WrappedResult & result);
};

}  // namespace qtexpression_controller
