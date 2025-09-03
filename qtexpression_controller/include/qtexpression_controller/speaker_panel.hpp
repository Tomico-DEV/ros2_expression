#pragma once

#include <rviz_common/panel.hpp>
#include <rviz_common/display_context.hpp>
#include <rviz_common/ros_integration/ros_node_abstraction_iface.hpp>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "speaker_actions/action/speak.hpp"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

namespace qtexpression_controller
{

using SpeakAction = speaker_actions::action::Speak;
using GoalHandleSpeak = rclcpp_action::ClientGoalHandle<SpeakAction>;

class SpeakerPanel : public rviz_common::Panel
{
Q_OBJECT
public:
    explicit SpeakerPanel(QWidget * parent=0);
    ~SpeakerPanel() override;

    void onInitialize() override;

protected:
    std::shared_ptr<rviz_common::ros_integration::RosNodeAbstractionIface> node_ptr_;
    rclcpp_action::Client<SpeakAction>::SharedPtr client_ptr_;
    rclcpp_action::ClientGoalHandle<SpeakAction>::SharedPtr active_goal_;

    // gui
    QLineEdit * input_text_;
    QLabel* viseme_;
    QLabel* result_;
    QPushButton* send_button_;
    QPushButton* cancel_button_;

private Q_SLOTS:
    void send_speak_goal_();
    void cancel_speak_goal_();
    
private:
    void create_layout_();
    void set_result_color_(Qt::GlobalColor color);
    void response_callback_(
        const GoalHandleSpeak::SharedPtr & goal_handle
    );
    void feedback_callback_(
        GoalHandleSpeak::SharedPtr,
        const std::shared_ptr<const SpeakAction::Feedback> feedback
    );
    void result_callback_(
        const GoalHandleSpeak::WrappedResult & result
    );
};

}