#pragma once

#include <algorithm>
#include <chrono>
#include <random>
#include <format>
#include <functional>
#include <filesystem>
#include <memory>
#include <map>
#include <mutex>
#include <optional>
#include <utility>
#include <string>

#include <iostream>

#include "ament_index_cpp/get_package_share_directory.hpp"
#include "behaviortree_cpp/bt_factory.h"

#include "tweeny/tweeny.h"

#include "expression/animator.hpp"

namespace expression
{

/**
 * \brief if there are any speaking tasks present
 */
class HasSpeakTask : public BT::ConditionNode
{
public:
  /**
   * \param name node name
   * \param config node config
   * \param check_func function to check if node is speaking
   */
  explicit HasSpeakTask(
    const std::string & name,
    const BT::NodeConfiguration & config,
    std::function<bool()> check_func);

  /**
   * \brief check if node has any speaking tasks
   */
  auto tick() -> BT::NodeStatus override;

private:
  std::function<bool()> check_func_;
};

class Breathe : public BT::SyncActionNode
{
public:
  enum class BreathState {
    INHALE,
    EXHALE
  };

  explicit Breathe(
    const std::string & name,
    const BT::NodeConfiguration & config,
    ChannelCore1D::SharedPtr p_diaphragm_chan,
    std::function<double()> get_bpm,
    std::function<double()> get_setpoint);

  auto tick() -> BT::NodeStatus override;

private:
  double setpoint_noise_();

  BreathState breath_state_{BreathState::INHALE};
  ChannelCore1D::SharedPtr p_chan_;
  std::function<double()> get_bpm_;
  std::function<double()> get_setpoint_;

  const double min_inhale_rate_ = 500;  // 0.5 secs
  const double min_exhale_rate_ = 500;  // 0.5 secs
  const double inhale_dur_ratio_ = 0.4;
  const double exhale_dur_ratio_ = 0.6;
  const double neutral_pos_ = 0.5;
  const double setpoint_stddev_ = 0.05;
};

class BreathAnimator : public Animator
{
public:
  void initialize(std::shared_ptr<ChanMap> p_chan_map) override;

  inline auto get_name() -> std::string override
  {
    return "SimpleBreathAnimator";
  }

  inline void handle_event(const Event &) override
  { }

  inline void update(std::chrono::milliseconds dt) override
  {
    if (bpm_tween_) {
      if (bpm_tween_->progress() >= 1.0f) {
        generate_bpm_tween_();
      }
      bpm_tween_val_ = bpm_tween_->step(static_cast<uint32_t>(dt.count()));
    }
    tree_.tickOnce();
  }

private:
  inline double get_bpm_()
  {
    return std::clamp<double>(bpm_tween_val_, 1, 60);
  }
  inline double get_target_setpoint_()
  {
    return setpoint_;
  }

  inline bool has_speak_task_()
  {
    return speak_event_;
  }

  void generate_bpm_tween_();

  // tween config
  const double bpm_tween_duration_ = 10000.0;  // 10 seconds
  const double average_bpm_ = 15.0;
  const double bpm_stddev_ = 1.2;

  BT::Tree tree_;
  double bpm_tween_val_ = 0.0;
  std::optional<tweeny::tween<double>> bpm_tween_;

  double setpoint_ = 0.25;
  std::mutex setpoint_mutex_;

  bool speak_event_ = false;
};

}  // namespace expression
