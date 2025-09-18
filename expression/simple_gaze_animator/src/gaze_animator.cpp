#include "simple_gaze_animator/gaze_animator.hpp"

#include <iostream>

namespace expression
{

void GazeAnimator::initialize(
  std::shared_ptr<ChanMap> p_chan_map, const ParamMap & param_map)
{
  p_engine_ = std::make_unique<std::mt19937>(std::random_device{}());

  p_left_eye_ = get_2d_chan_(*p_chan_map, "left_eye/eye");
  p_right_eye_ = get_2d_chan_(*p_chan_map, "right_eye/eye");

  auto get_param =
    [&](const std::string& param_name, double & target) -> void
    {
      try {
        target = std::stod(param_map.at(param_name));
      } catch (const std::exception & e) {
        RCLCPP_INFO(
          rclcpp::get_logger("expression"),
          "Param not found for %s", param_name.c_str());
      }
    };

  get_param("/eye/horizontal/max", max_eye_h_angle_);
  get_param("/eye/horizontal/min", min_eye_h_angle_);
  get_param("/eye/vertical/max", max_eye_v_angle_);
  get_param("/eye/vertical/min", min_eye_v_angle_);
  get_param("/eye/center_dist", eye_center_dist_);
  get_param("/saccade/angle/max", saccade_angle_max_);
  get_param("/saccade/angle/stddev", saccade_angle_stddev_);
  get_param("/saccade/start_delay", saccade_start_delay_);
  get_param("/saccade/interval/min", saccade_interval_min_);
  get_param("/saccade/interval/max", saccade_interval_max_);
  get_param("/saccade/interval/avg", saccade_interval_avg_);
  get_param("/saccade/interval/stddev", saccade_interval_stddev_);

  // input checking
  if (saccade_angle_max_ < 0.0) {
    RCLCPP_ERROR(
      rclcpp::get_logger("expression"),
      "Saccade max angle can't be negative!");
  }

  if (
    ((max_eye_h_angle_ - min_eye_h_angle_) < 2 * saccade_angle_max_) ||
    ((max_eye_v_angle_ - min_eye_v_angle_) < 2 * saccade_angle_max_))
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("expression"),
      "Eye angle range too small for saccade padding!");
  }
}

void GazeAnimator::handle_event(const Event & e)
{
  if (e.type == Event::Type::GAZE) {
    auto gaze_map =
      std::any_cast<EventDataType_t<Event::Type::GAZE>>(e.data);

    if (gaze_map.size() > 0) {
      // no gaze switching functionality - just use last member
      auto it = gaze_map.end();
      it--;
      if (it->second != prev_gaze) {
        eye_last_update_ = std::chrono::milliseconds{0};
        prev_gaze = it->second;
      }
      calculate_eye_angles_(it->second);
    }
  }
}

void GazeAnimator::update(std::chrono::milliseconds dt)
{
  eye_last_update_ += dt;
  if (eye_last_update_.count() > saccade_start_delay_) {
    if (stop_saccade_) {
      stop_saccade_ = false;
      saccade_now_ = std::chrono::milliseconds{0};
      saccade_next_ = std::chrono::milliseconds{0};
    }
    update_saccade_(dt);
  } else {
    stop_saccade_ = true;
    saccade_offset_.pitch = 0.0;
    saccade_offset_.yaw = 0.0;
  }

  if (p_left_eye_->done()) {
    p_left_eye_->set_tween(
      tweeny::from<Vec2D>(p_left_eye_->get_current())
        .to(Vec2D{
          left_angles_.yaw + saccade_offset_.yaw,
          left_angles_.pitch + saccade_offset_.pitch})
        .during(dt.count())
        .via(tweeny::easing::sinusoidalInOut));
  }
  if (p_right_eye_->done()) {
    p_right_eye_->set_tween(
    tweeny::from<Vec2D>(p_right_eye_->get_current())
      .to(Vec2D{
        right_angles_.yaw + saccade_offset_.yaw,
        right_angles_.pitch + saccade_offset_.pitch})
      .during(dt.count())
      .via(tweeny::easing::sinusoidalInOut));
  }
}
/**
 * \brief Calcuate eye angles from give gaze vector
 * 
 * Stores calculate values to members left_angles_ and right_angles_
 */
void GazeAnimator::calculate_eye_angles_(Vec3D gaze)
{
  /*
    * gaze frame
                              * gaze frame
               +x     O
    O * O   +y_|                     +z
    face                              |_+x
    frame

    0 deg = straight forward
    - deg = left / down
    + deg = right / up
  */

  double min_eye_h_padded = min_eye_h_angle_ + saccade_angle_max_;
  double max_eye_h_padded = max_eye_h_angle_ - saccade_angle_max_;
  double min_eye_v_padded = min_eye_v_angle_ + saccade_angle_max_;
  double max_eye_v_padded = max_eye_v_angle_ - saccade_angle_max_;

  left_angles_.yaw =
    std::clamp(
      atan2(gaze.y - eye_center_dist_ / 2, gaze.x),
      min_eye_h_padded, max_eye_h_padded);
  left_angles_.pitch =
    std::clamp(
      atan2(gaze.z, gaze.x),
      min_eye_v_padded, max_eye_v_padded);
  right_angles_.yaw =
    std::clamp(
      atan2(gaze.y + eye_center_dist_ / 2, gaze.x),
      min_eye_h_padded, max_eye_h_padded);
  right_angles_.pitch = left_angles_.pitch;
}

void GazeAnimator::update_saccade_(std::chrono::milliseconds dt)
{
  saccade_now_ += dt;
  if (saccade_now_ >= saccade_next_) {
    std::normal_distribution<double>
      angle_distribution{0.0, saccade_angle_stddev_};
    double yaw = angle_distribution(*p_engine_);
    double pitch = angle_distribution(*p_engine_);
    saccade_offset_.yaw =
      std::clamp(yaw, -saccade_angle_max_, saccade_angle_max_);
    saccade_offset_.pitch =
      std::clamp(pitch, -saccade_angle_max_, saccade_angle_max_);

    std::normal_distribution<double>
      interval_distribution{
        saccade_interval_avg_, saccade_interval_stddev_};
    double interval = interval_distribution(*p_engine_);
    saccade_next_ = std::chrono::milliseconds{
      static_cast<uint>(
        std::clamp(interval,
          saccade_interval_min_, saccade_interval_max_))};
    saccade_now_ = std::chrono::milliseconds{0};
  }
}

}  // namespace expression

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(expression::GazeAnimator, expression::Animator)
