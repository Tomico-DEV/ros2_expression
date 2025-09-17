#pragma once

#include <cmath>
#include <memory>
#include <string>

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

  ChannelCore2D::SharedPtr p_left_eye_;
  ChannelCore2D::SharedPtr p_right_eye_;

  // parameters
  double max_eye_h_angle_;
  double min_eye_h_angle_;
  double max_eye_v_angle_;
  double min_eye_v_angle_;
  double eye_center_dist_;

  EyeAngles left_angles_;
  EyeAngles right_angles_;
};

}  // namespace expression
