#pragma once

#include <algorithm>
#include <cmath>
#include <chrono>
#include <random>
#include <memory>
#include <string>

#include "rclcpp/logging.hpp"

#include "tweeny/tweeny.h"

#include "expression/animator.hpp"

namespace expression
{

struct EyeAngles
{
  double pitch = 0.0;
  double yaw = 0.0;
};

class GazeAnimator : public Animator
{
public:
  void initialize(
    std::shared_ptr<ChanMap> p_chan_map,
    const ParamMap & param_map) override;

  inline auto get_name() -> std::string override
  {
    return "simple_gaze_animator";
  }

  void handle_event(const Event & e) override;

  void update(std::chrono::milliseconds dt) override;

private:
  void calculate_eye_angles_(Vec3D gaze);
  void update_saccade_(std::chrono::milliseconds dt);

  ChannelCore2D::SharedPtr p_left_eye_;
  ChannelCore2D::SharedPtr p_right_eye_;

  // parameters
  double max_eye_h_angle_ = M_PI / 6.0;
  double min_eye_h_angle_ = -M_PI / 6.0;
  double max_eye_v_angle_ = M_PI / 6.0;
  double min_eye_v_angle_ = - M_PI / 6.0;
  double eye_center_dist_ = 0.15;
  double saccade_angle_max_ = 0.08;
  double saccade_angle_stddev_ = 0.03;
  double saccade_start_delay_ = 500;
  double saccade_interval_avg_ = 300;  // units in ms
  double saccade_interval_stddev_ = 75;
  double saccade_interval_min_ = 50;
  double saccade_interval_max_ = 1000;

  std::chrono::milliseconds saccade_now_{0};
  std::chrono::milliseconds saccade_next_{0};

  bool stop_saccade_ = true;
  std::chrono::milliseconds eye_last_update_{0};

  EyeAngles saccade_offset_;

  EyeAngles left_angles_;
  EyeAngles right_angles_;

  Vec3D prev_gaze;

  std::unique_ptr<std::mt19937> p_engine_;
};

}  // namespace expression
