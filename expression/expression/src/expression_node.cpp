#include "expression/expression_node.hpp"

// overload trick
template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

namespace expression
{

ExpressionNode::ExpressionNode(const rclcpp::NodeOptions &options)
: rclcpp_lifecycle::LifecycleNode{"expression", options},
  p_chan_map_{std::make_shared<ChanMap>()}
{
  declare_params_();
}

/**
 * \brief create publishers, channels, and animators
 */
auto ExpressionNode::on_configure(const rclcpp_lifecycle::State &)
-> CallbackReturn
{
  RCLCPP_INFO(get_logger(), "Configurating...");

  try {
    create_channels_();
    create_animators_();

    return CallbackReturn::SUCCESS;
  } catch (const std::exception& e) {
    RCLCPP_ERROR(get_logger(), "Failed to configure: %s", e.what());
  }

  return CallbackReturn::FAILURE;
}

auto ExpressionNode::on_activate(const rclcpp_lifecycle::State &)
-> CallbackReturn
{
  RCLCPP_INFO(get_logger(), "Activating..");

  try {
    start_channels_();
    start_animation_();

    return CallbackReturn::SUCCESS;
  } catch (const std::exception& e) {
    RCLCPP_ERROR(get_logger(), "Failed to activate: %s", e.what());
  }

  return CallbackReturn::FAILURE;
}

/**
 * \brief stop animators and channels
 */
auto ExpressionNode::on_deactivate(const rclcpp_lifecycle::State &)
-> CallbackReturn
{
  stop_animation_();
  stop_channels_();

  return CallbackReturn::SUCCESS;
}

/**
 * \brief stop animators, channels, and cleanup resources
 */
auto ExpressionNode::on_cleanup(const rclcpp_lifecycle::State &)
-> CallbackReturn
{
  RCLCPP_INFO(get_logger(), "Cleaning up...");

  try {
    stop_animation_();
    stop_channels_();

    p_left_eye_chans_.reset();
    p_right_eye_chans_.reset();
    p_mouth_chans_.reset();
    p_neck_chans_.reset();

    p_chan_map_.reset();
    animators_.clear();

    // no implementation
    p_speak_client_.reset();
    p_tts_server_.reset();

    animator_loader_.reset();

    return CallbackReturn::SUCCESS;
  } catch (const std::exception& e) {
    RCLCPP_ERROR(get_logger(), "Failed to clean up: %s", e.what());
  }

  return CallbackReturn::FAILURE;
}

auto ExpressionNode::on_shutdown(const rclcpp_lifecycle::State & state)
-> CallbackReturn
{
  RCLCPP_INFO(get_logger(), "Shutting down from state %s", state.label().c_str());

  stop_animation_();
  stop_channels_();

  return CallbackReturn::SUCCESS;
}

void ExpressionNode::declare_params_()
{
  RCLCPP_INFO(get_logger(), "Declared params");
  using ParamDesc = rcl_interfaces::msg::ParameterDescriptor;
  // using IntRange = rcl_interfaces::msg::IntegerRange;
  using fpath = std::filesystem::path;

  // convenience function for making parameter description
  auto make_desc =
    [&](const std::string & desc_text)
    {
      ParamDesc desc;
      desc.description = desc_text;
      return desc;
    };

  animator_plugins_str_ = declare_parameter<std::vector<std::string>>(
    "animator_plugins", {"expression::BreathAnimator", "expression::BlinkAnimator"},
    make_desc("Animators to load"));
}

void ExpressionNode::create_channels_()
{
  p_left_eye_chans_ =
    std::make_shared<EyeChannels>(
      this, "left_eye", 10,
      p_chan_map_, animate_rate_);
  p_right_eye_chans_ =
    std::make_shared<EyeChannels>(
      this, "right_eye", 10,
      p_chan_map_, animate_rate_);
  p_mouth_chans_ =
    std::make_shared<MouthChannels>(
      this, 10, p_chan_map_, animate_rate_);
  p_neck_chans_ =
    std::make_shared<NeckChannels>(
      this, 10, p_chan_map_, animate_rate_);

  RCLCPP_INFO(get_logger(), "Made Channels");
}

void ExpressionNode::create_animators_()
{
  animator_loader_ =
    std::make_unique<pluginlib::ClassLoader<Animator>>("expression", "expression::Animator");
  for (const auto & animator_str : animator_plugins_str_) {
    Animator::SharedPtr animator;
    try {
      animator = animator_loader_->createSharedInstance(animator_str);
      animator->initialize(p_chan_map_);
      animators_.push_back(animator);
    } catch (const std::exception& e) {
      throw std::runtime_error{
        std::format(
          "Failed to make animator {}: {}. Has the animator been built?",
          animator_str, e.what())};
    }
    RCLCPP_INFO(get_logger(), "Loaded animator: %s", animator->get_name().c_str());
  }
}

void ExpressionNode::start_channels_()
{
  RCLCPP_INFO(get_logger(), "Starting channels..");
  p_left_eye_chans_->start();
  p_right_eye_chans_->start();
  p_mouth_chans_->start();
  p_neck_chans_->start();
}

void ExpressionNode::stop_channels_()
{
  p_left_eye_chans_->stop();
  p_right_eye_chans_->stop();
  p_mouth_chans_->stop();
  p_neck_chans_->stop();

  RCLCPP_INFO(get_logger(), "Stopped channels");
}

void ExpressionNode::start_animation_()
{
  RCLCPP_INFO(get_logger(), "Starting animators..");

  bool expected = false;
  if (!running_.compare_exchange_strong(expected, true)) {
    RCLCPP_WARN(get_logger(), "Can't start; Animators already running!");
    return;  // thread is already running
  }
  animator_thread_ =
    std::jthread{std::bind(&ExpressionNode::update_animation_, this)};
}

void ExpressionNode::stop_animation_()
{
  bool expected = true;
  if (!running_.compare_exchange_strong(expected, false)) {
    RCLCPP_WARN(get_logger(), "Can't stop; Animators already stopped!");
    return;  // thread is already stopped
  }

  if (animator_thread_.joinable()) {
    animator_thread_.join();
  }

  RCLCPP_INFO(get_logger(), "Stopped animators");
}

void ExpressionNode::update_animation_()
{
  while (running_.load()) {
    for (const auto & animator : animators_) {
      animator->update(animate_rate_);
    }

    std::this_thread::sleep_for(animate_rate_);
  }
}

}  // namespace expression

RCLCPP_COMPONENTS_REGISTER_NODE(expression::ExpressionNode)
