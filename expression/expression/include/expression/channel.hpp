#pragma once

#include <atomic>
#include <chrono>
#include <thread>
#include <memory>
#include <optional>

#include <iostream>

#include "tweeny/tweeny.h"

#include "face_msgs/msg/param1_d.hpp"
#include "face_msgs/msg/param2_d.hpp"

#include "rclcpp/rclcpp.hpp"

#include "expression/utils.hpp"
#include "expression/channel_core.hpp"

namespace expression
{

// forward declare
class GroupedChannels;

template <typename PubT, typename TweenT>
class Channel : public ChannelCore<TweenT>
{
  friend GroupedChannels;

public:
  using SharedPtr = std::shared_ptr<Channel<PubT, TweenT>>;

  explicit Channel(PubT pub);

  void start() override;
  void stop() override;

  inline bool is_running() override
  {
    return playing_.load();
  }

protected:
  using ChannelCore<TweenT>::t_;

  void step_(std::chrono::milliseconds rate);

  virtual void tween2pub(TweenT val) = 0;


  PubT pub_;

private:
  std::atomic_bool playing_{false};
};

class Channel1D : public Channel<Pub1D, double>
{
public:
  explicit inline Channel1D(Pub1D pub)
  : Channel{pub}
  { }

protected:
  inline void tween2pub(double val) override
  {
    Param1D msg;
    msg.val = val;
    if (pub_) {
      pub_->publish(msg);
    }
  }
};

class Channel2D : public Channel<Pub2D, Vec2D>
{
public:
  explicit inline Channel2D(Pub2D pub)
  : Channel{pub}
  { }

protected:
  inline void tween2pub(Vec2D val) override
  {
    Param2D msg;
    msg.x = val.x;
    msg.y = val.y;
    if (pub_) {
      pub_->publish(msg);
    }
  }
};

// templated defs

template <typename PubT, typename TweenT>
Channel<PubT, TweenT>::Channel(PubT pub)
: pub_{pub}
{ }

template <typename PubT, typename TweenT>
void Channel<PubT, TweenT>::start()
{
  playing_.store(true);
}

template <typename PubT, typename TweenT>
void Channel<PubT, TweenT>::stop()
{
  playing_.store(false);
  std::lock_guard lock{this->tween_mut_};
  if (t_) {
      t_.reset();  // deterministic destruction
  }
}

template <typename PubT, typename TweenT>
void Channel<PubT, TweenT>::step_(std::chrono::milliseconds rate)
{
  if (playing_.load()) {
    // update tween and publish
    if (t_) {
      tween2pub(t_->step(static_cast<int32_t>(rate.count())));
    }
  }
}


}  // namespace expression
