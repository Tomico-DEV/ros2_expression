#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <random>

#include "behaviortree_cpp/bt_factory.h"

#include "tweeny/tweeny.h"

#include "ament_index_cpp/get_package_share_directory.hpp"

#include "expression/animator.hpp"

namespace expression
{

class HasStareEvent : public BT::ConditionNode
{
public:
  /**
   * \param name node name
   * \param config node config
   * \param check_func function to check for stare events
   */
  explicit HasStareEvent(
    const std::string & name,
    const BT::NodeConfiguration & config,
    std::function<bool()> check_func);

  auto tick() -> BT::NodeStatus override;

private:
  std::function<bool()> check_func_;
};

class Blink : public BT::SyncActionNode
{
public:
  explicit Blink(
    const std::string & name,
    const BT::NodeConfiguration & config,
    ChannelCore1D::SharedPtr p_left_eyelid,
    ChannelCore1D::SharedPtr p_right_eyelid);

  auto tick() -> BT::NodeStatus override;

private:
  double generate_interval_();
  double generate_blink_dur_();
  double interval_noise_();
  tweeny::tween<double> add_blink_(tweeny::tween<double> tween, double blink_dur);
  tweeny::tween<double> add_interval_(tweeny::tween<double> tween, double interval);

  ChannelCore1D::SharedPtr p_left_eyelid_;
  ChannelCore1D::SharedPtr p_right_eyelid_;

  std::mt19937 engine_;

  // units are in ms
  const double interval_stddev_ = 1000;
  const double noise_stddev_ = 20;
  const double max_interval_ = 10000.0;  // 10 secs
  const double min_interval_ = 500;
  const double avg_interval_ = 60.0 / 17.0 * 1000.0;
  const double avg_blink_duration_ = 300.0;
  const double blink_duration_stddev_ = 10.0;
  const double blink_stride_ratio_ = 0.3;
  const double blink_close_ratio_ = 0.2;
};

class BlinkAnimator : public Animator
{
public:
  void initialize(std::shared_ptr<ChanMap> p_chan_map) override;

  inline auto get_name() -> std::string override
  {
    return "SimpleBlinkAnimator";
  }

  inline void handle_event(const Event &) override
  { }

  inline void update(std::chrono::milliseconds dt) override
  {
    tree_.tickOnce();
  }

private:
  inline bool has_stare_event_()
  {
    return stare_event_;
  }


  BT::Tree tree_;

  bool stare_event_ = false;
};

}  // namespace expression
