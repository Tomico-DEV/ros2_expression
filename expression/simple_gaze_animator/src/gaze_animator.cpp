#include "simple_gaze_animator/gaze_animator.hpp"

#include <iostream>

namespace expression
{

void GazeAnimator::initialize(
  std::shared_ptr<ChanMap> p_chan_map, const ParamMap & param_map)
{
  p_left_eye_ = get_2d_chan_(*p_chan_map, "left_eye/eye");
  p_right_eye_ = get_2d_chan_(*p_chan_map, "right_eye/eye");

  auto get_param =
    [&](const std::string& param_name) -> double
    {
      try {
        return std::stod(param_map.at(param_name));
      } catch (const std::exception & e) {
        std::cout << "Param not found for " << param_name << "!\n";
        return 0.0;
      }
    };

  max_eye_h_angle_ = get_param("/eye/horizontal/max");
  min_eye_h_angle_ = get_param("/eye/horizontal/min");
  max_eye_v_angle_ = get_param("/eye/vertical/max");
  min_eye_v_angle_ = get_param("/eye/vertical/min");
  eye_center_dist_ = get_param("/eye/center_dist");
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
      calculate_eye_angles_(it->second);
    }
  }
}

void GazeAnimator::update(std::chrono::milliseconds dt)
{
  if (p_left_eye_->done()) {
    p_left_eye_->set_tween(
      tweeny::from<Vec2D>(p_left_eye_->get_current())
        .to(Vec2D{left_angles_.yaw, left_angles_.pitch})
        .during(dt.count())
        .via(tweeny::easing::sinusoidalInOut));
  }
  if (p_right_eye_->done()) {
    p_right_eye_->set_tween(
    tweeny::from<Vec2D>(p_right_eye_->get_current())
      .to(Vec2D{right_angles_.yaw, right_angles_.pitch})
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

  left_angles_.yaw = atan2(gaze.y + eye_center_dist_ / 2, gaze.x);
  left_angles_.pitch = atan2(gaze.z, gaze.x);
  right_angles_.yaw = atan2(gaze.y - eye_center_dist_ / 2, gaze.x);
  right_angles_.pitch = left_angles_.pitch;
}

}  // namespace expression

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(expression::GazeAnimator, expression::Animator)
