#include "qtexpression_controller/speaker_panel.hpp"

namespace qtexpression_controller
{

SpeakerPanel::SpeakerPanel(QWidget * parent)
: Panel(parent)
{ 
    create_layout_();
}

SpeakerPanel::~SpeakerPanel() = default;

void SpeakerPanel::onInitialize()
{
    // get the abstract node and lock it while we do stuff
    node_ptr_ = getDisplayContext()->getRosNodeAbstraction().lock();
    
    rclcpp::Node::SharedPtr node = node_ptr_->get_raw_node();

    client_ptr_ = rclcpp_action::create_client<SpeakAction>(
        node,
        "speak"
    );
}


void SpeakerPanel::create_layout_()
{
    const auto layout = new QVBoxLayout { this };
    auto * gridlayout = new QGridLayout;

    // text input field
    const auto input_label = new QLabel { "Text:" };
    input_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    input_text_ = new QLineEdit { "おはよう世界" };
    gridlayout->addWidget(input_label, 0, 0);
    gridlayout->addWidget(input_text_, 0, 1);
    
    // feedback output field
    const auto viseme_label = new QLabel { "Viseme:" };
    viseme_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    viseme_ = new QLabel { "" };
    gridlayout->addWidget(viseme_label, 1, 0);
    gridlayout->addWidget(viseme_, 1, 1);

    // result output field
    const auto result_label = new QLabel { "Result:" };
    result_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    result_ = new QLabel { "" };
    gridlayout->addWidget(result_label, 2, 0);
    gridlayout->addWidget(result_, 2, 1);
    
    send_button_ = new QPushButton { "Send Goal" };
    cancel_button_ = new QPushButton { "Cancel" };

    layout->addLayout(gridlayout);
    layout->addWidget(send_button_);
    layout->addWidget(cancel_button_);

    QObject::connect(send_button_, &QPushButton::released, this, &SpeakerPanel::send_speak_goal_);
    QObject::connect(cancel_button_, &QPushButton::released, this, &SpeakerPanel::cancel_speak_goal_);
}

void SpeakerPanel::set_result_color_(Qt::GlobalColor color)
{
    QPalette palette = result_->palette();
    palette.setColor(QPalette::WindowText, color);
    result_->setPalette(palette);
}


void SpeakerPanel::send_speak_goal_()
{
    std::string goal_text = input_text_->text().toStdString();

    RCLCPP_INFO(
        rclcpp::get_logger("SpeakerPanel"), 
        "Sending goal: %s", goal_text.c_str()
    );
    
    if (!client_ptr_->action_server_is_ready())
    {
        RCLCPP_INFO(
            rclcpp::get_logger("SpeakerPanel"), 
            "Action server is not available"
        );

        result_->setText("Sever not avail");
        set_result_color_(Qt::red);
        
        return;
    }
    
    
    auto goal_msg = SpeakAction::Goal();
    goal_msg.text= goal_text;

    auto send_goal_options = rclcpp_action::Client<SpeakAction>::SendGoalOptions();
    send_goal_options.goal_response_callback = 
        std::bind(
            &SpeakerPanel::response_callback_, 
            this, 
            std::placeholders::_1
        );
    send_goal_options.feedback_callback = 
        std::bind(
            &SpeakerPanel::feedback_callback_, 
            this, 
            std::placeholders::_1,
            std::placeholders::_2
        );
    send_goal_options.result_callback =
        std::bind(
            &SpeakerPanel::result_callback_,
            this,
            std::placeholders::_1
        );
    
    client_ptr_->async_send_goal(goal_msg, send_goal_options);

    result_->setText("Goal sent..");
    set_result_color_(Qt::black);
}

void SpeakerPanel::cancel_speak_goal_()
{
    if (!active_goal_) {
        RCLCPP_INFO(rclcpp::get_logger("SpeakerPanel"), "No active goal to cancel");
        result_->setText("No active goal");
        set_result_color_(Qt::red);
        return;
    }

    auto future_cancel = client_ptr_->async_cancel_goal(active_goal_);
    result_->setText("Canceling..");
    set_result_color_(Qt::black);
}

void SpeakerPanel::response_callback_(
    const GoalHandleSpeak::SharedPtr & goal_handle
)
{
    if (!goal_handle)
    {
        result_->setText("Goal Rejected");
        set_result_color_(Qt::red);
    }
    else
    {
        result_->setText("Goal Accepted");
        set_result_color_(Qt::black);
    }

    active_goal_ = goal_handle;
}

void SpeakerPanel::feedback_callback_(
    GoalHandleSpeak::SharedPtr,
    const std::shared_ptr<const SpeakAction::Feedback> feedback
)
{
    viseme_->setText(QString::fromStdString(feedback->viseme));
}

void SpeakerPanel::result_callback_(
    const GoalHandleSpeak::WrappedResult & result
)
{
    switch (result.code) {
        case rclcpp_action::ResultCode::SUCCEEDED:
            break;
        case rclcpp_action::ResultCode::ABORTED:
            result_->setText("Aborted");
            set_result_color_(Qt::red);
            break;
        case rclcpp_action::ResultCode::CANCELED:
            result_->setText("Canceled");
            set_result_color_(Qt::red);
            break;
        default:
            result_->setText("Unknown");
            set_result_color_(Qt::red);
            return;
    }
    
    if (result.result->success)
    {
        result_->setText("Ok");
        set_result_color_(Qt::green);
    }
    else
    {
        result_->setText("Failed");
        set_result_color_(Qt::red);
    }

    active_goal_.reset();
}

}


#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(qtexpression_controller::SpeakerPanel, rviz_common::Panel)