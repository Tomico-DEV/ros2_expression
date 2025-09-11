#include "expression/expression_node.hpp"


namespace expression
{

ExpressionNode::ExpressionNode(const rclcpp::NodeOptions &options)
: Node{"expression", options}
{
  create_publishers_();
  create_animators_();
  start_channels_();
  start_animation_();
}

ExpressionNode::~ExpressionNode()
{
  stop_channels_();
  stop_animation_();
}

void ExpressionNode::declare_params_()
{
  using ParamDesc = rcl_interfaces::msg::ParameterDescriptor;
  using IntRange = rcl_interfaces::msg::IntegerRange;
  using fpath = std::filesystem::path;

  // convenience function for making parameter description
  auto make_desc =
    [&](const std::string & desc_text)
    {
      ParamDesc desc;
      desc.description = desc_text;
      return desc;
    };

  // bt parameters
  fpath package_path = ament_index_cpp::get_package_share_directory("expression");
}

void ExpressionNode::create_publishers_()
{
  p_left_eye_pubs_ = std::make_shared<EyePublishers>(this, "left_eye", 10);
  p_right_eye_pubs_ = std::make_shared<EyePublishers>(this, "right_eye", 10);
  p_mouth_pubs_ = std::make_shared<MouthPublishers>(this, 10);
  p_neck_pubs_ = std::make_shared<NeckPublishers>(this, 10);

  p_diaphragm = std::make_shared<Channel1D>(
    create_publisher<Param1D>("diaphragm", 10),
    animate_rate_);

  RCLCPP_INFO(get_logger(), "Made publishers");
}

void ExpressionNode::create_animators_()
{
  using fpath = std::filesystem::path;

  fpath package_path = ament_index_cpp::get_package_share_directory("expression");
  fpath breath_anim_path = package_path / "behavior_trees" / "breath_anim.xml";

  p_breath_animator =
    std::make_unique<BreathAnimator>(
      std::bind(&ExpressionNode::has_speak_task_, this),
      p_diaphragm, breath_anim_path);

  RCLCPP_INFO(get_logger(), "Made animators");
}

void ExpressionNode::start_channels_()
{
  RCLCPP_INFO(get_logger(), "Starting channels..");
  p_diaphragm->start();
}

void ExpressionNode::stop_channels_()
{
  p_diaphragm->stop();
}

void ExpressionNode::start_animation_()
{
  RCLCPP_INFO(get_logger(), "Starting animation");

  running_.store(true);
  animator_thread_ =
    std::jthread{std::bind(&ExpressionNode::update_animation_, this)};
}

void ExpressionNode::stop_animation_()
{
  running_.store(false);
  if (animator_thread_.joinable()) {
    animator_thread_.join();
  }
}

void ExpressionNode::update_animation_()
{
  while (running_.load()) {
    p_breath_animator->update(animate_rate_);

    std::this_thread::sleep_for(animate_rate_);
  }
}

}  // namespace expression

RCLCPP_COMPONENTS_REGISTER_NODE(expression::ExpressionNode)
