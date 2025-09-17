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
  p_chan_map_{std::make_shared<ChanMap>()},
  tf_buffer_{get_clock()}, tf_listener_{tf_buffer_}
{
  declare_params_();
}

ExpressionNode::~ExpressionNode()
{
  cleanup_();
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
    cleanup_();
  } catch (const std::exception& e) {
    RCLCPP_ERROR(get_logger(), "Failed to clean up: %s", e.what());
    return CallbackReturn::FAILURE;
  }

  return CallbackReturn::SUCCESS;
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
    "animator_plugins",
    {  // default animators
      "expression::BreathAnimator",
      "expression::BlinkAnimator",
      "expression::GazeAnimator"},
    make_desc("Animators to load"));

  face_frame_ = declare_parameter<std::string>(
    "face_frame", "face", make_desc("name of head frame"));
  gaze_prefix_ = declare_parameter<std::string>(
    "gaze_prefix", "gaze", make_desc("gaze tfs prefix"));

  fpath launch_pkg_path =
    ament_index_cpp::get_package_share_directory("expression_launch");
  fpath default_anim_config_path =
    launch_pkg_path / "config" / "animator_config.yaml";
  std::string anim_config_pathstr = declare_parameter<std::string>(
    "animator_config", default_anim_config_path.string(),
    make_desc("Filepath to animator config yaml"));
  anim_config_path_ = fpath{anim_config_pathstr};
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
  YAML::Node anim_config = YAML::LoadFile(
    anim_config_path_.string())["animators"];

  animator_loader_ =
    std::make_unique<pluginlib::ClassLoader<Animator>>("expression", "expression::Animator");
  for (const auto & animator_str : animator_plugins_str_) {
    Animator::SharedPtr animator;
    try {
      animator = animator_loader_->createSharedInstance(animator_str);
      // get params
      Animator::ParamMap param_map;
      std::string anim_name = animator->get_name();
      try {
        auto anim_params = anim_config[anim_name];
        if (anim_params) {
          flatten_mappings_(anim_params, param_map, "");
        } else {
          RCLCPP_WARN(
            get_logger(), "Skipping params for %s (not found)", anim_name.c_str());
        }
      } catch (const std::exception& e) {
        RCLCPP_ERROR(
          get_logger(),
          "Failed to parse animator %s params!: %s",
          anim_name.c_str(), e.what());
      }
      animator->initialize(p_chan_map_, param_map);
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

/**
 * \brief convert nested mappings specified in puppet's yaml into
 *        a map of string paths
 * \param [in] node root node to search from
 * \param [out] out_map output map to be populated
 * \param [in] current_path string path of provided root node
 */
void ExpressionNode::flatten_mappings_(
  const YAML::Node & node,
  std::map<std::string, std::string> & out_map,
  const std::string & current_path
)
{
  if (!node.IsMap()) {
    return;
  }

  // node is a map - get key and valueuint32 style
  for (auto & entry : node) {
    std::string key = entry.first.as<std::string>();
    const YAML::Node & value = entry.second;

    // append key to current path
    std::string new_path = current_path + "/" + key;

    if (value.IsMap()) {
      flatten_mappings_(value, out_map, new_path);  // keep going..
    } else if (value.IsScalar()) {
      out_map[new_path] = value.as<std::string>();  // reached end, add key and val to map!
    } else {
      throw std::runtime_error(
        "Unsupported value in animator_config! Must be map or scalar");
    }
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

/**
 * \brief publish events and update animators
 */
void ExpressionNode::update_animation_()
{
  while (running_.load()) {
    // get gaze transforms
    std::vector<std::string> gaze_frames;
    bool face_frame_present = false;
    for (const auto & frame : tf_buffer_.getAllFrameNames()) {
      if (frame == face_frame_) {
        face_frame_present = true;
      }
      if (frame.starts_with(gaze_prefix_)) {
        gaze_frames.push_back(frame);
      }
    }
    // get head to gaze transform
    std::map<std::string, Vec3D> gaze_map;
    if (face_frame_present) {
      for (const auto & gaze_frame : gaze_frames) {
        try {
          geometry_msgs::msg::TransformStamped t;
          t = tf_buffer_.lookupTransform(
            face_frame_, gaze_frame, tf2::TimePointZero);
          // transform origin to see where it ends up

          geometry_msgs::msg::PointStamped origin;
          geometry_msgs::msg::PointStamped origin_transformed;
          origin.header.frame_id = t.header.frame_id;
          origin.point.x = 0.0;
          origin.point.y = 0.0;
          origin.point.z = 0.0;
          tf2::doTransform(origin, origin_transformed, t);

          Vec3D gaze_vec {
            origin_transformed.point.x,
            origin_transformed.point.y,
            origin_transformed.point.z};

          gaze_map[gaze_frame] = gaze_vec;
        } catch (const std::exception& e) {
          RCLCPP_ERROR(
            get_logger(),
            "Failed to get gaze_transform (%s): %s",
            gaze_frame.c_str(), e.what());
        }
      }
    }

    for (const auto & animator : animators_) {
      // gaze event
      if (gaze_map.size() > 0) {
        animator->handle_event(
          Event{Event::Type::GAZE, gaze_map});
      }
      animator->update(animate_rate_);
    }

    std::this_thread::sleep_for(animate_rate_);
  }
}

void ExpressionNode::cleanup_()
{
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
}

}  // namespace expression

RCLCPP_COMPONENTS_REGISTER_NODE(expression::ExpressionNode)
