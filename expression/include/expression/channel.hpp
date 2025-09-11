#pragma once

#include <atomic>
#include <chrono>
#include <thread>
#include <memory>
#include <mutex>
#include <optional>

#include "tweeny.h"

#include "face_msgs/msg/param1_d.hpp"
#include "face_msgs/msg/param2_d.hpp"

#include "rclcpp/rclcpp.hpp"

#include "expression/utils.hpp"

namespace expression
{

template <typename PubT, typename TweenT>
class Channel
{
public:
  using SharedPtr = std::shared_ptr<Channel<PubT, TweenT>>;

  explicit Channel(PubT pub, std::chrono::milliseconds rate);

  void start();
  void stop();

  inline bool is_running()
  {
    return playing_.load();
  }
  inline bool done()
  {
    std::lock_guard lock{tween_mut_};
    if (!t_.has_value()) {
      return true;
    }

    return t_->progress() >= 1.0f;
  }
  inline void set_tween(tweeny::tween<TweenT> tween)
  {
    std::lock_guard lock{tween_mut_};
    t_ = tween;
  }
  inline TweenT get_current()
  {
    std::lock_guard lock{tween_mut_};
    if (t_) {
      return t_->seek(t_->progress());
    }
    return TweenT{};  // default construct
  }
protected:
  virtual void tween2pub(TweenT val) = 0;

  PubT pub_;
  std::optional<tweeny::tween<TweenT>> t_;
private:
  void run_();

  std::mutex tween_mut_;
  std::atomic_bool playing_{false};
  std::chrono::milliseconds rate_;
  std::jthread play_thread_;
};

class Channel1D : public Channel<Pub1D, double>
{
public:
  explicit inline Channel1D(Pub1D pub, std::chrono::milliseconds rate)
  : Channel{pub, rate}
  { }

protected:
  inline void tween2pub(double val) override
  {
    Param1D msg;
    msg.val = val;
    pub_->publish(msg);
  }
};

struct Vec2D
{
  double x, y;

  Vec2D operator+(const Vec2D& o) const { return {x + o.x, y + o.y}; }
  Vec2D operator-(const Vec2D& o) const { return {x - o.x, y - o.y}; }
  Vec2D operator*(double d) const { return {x * d, y * d}; }
};

class Channel2D : public Channel<Pub2D, Vec2D>
{
public:
  explicit inline Channel2D(Pub2D pub, std::chrono::milliseconds rate)
  : Channel{pub, rate}
  { }

protected:
  inline void tween2pub(Vec2D val) override
  {
    Param2D msg;
    msg.x = val.x;
    msg.y = val.y;
    pub_->publish(msg);
  }
};

// templated defs

template <typename PubT, typename TweenT>
Channel<PubT, TweenT>::Channel(PubT pub, std::chrono::milliseconds rate)
: pub_{pub}, rate_{rate}
{ }

template <typename PubT, typename TweenT>
void Channel<PubT, TweenT>::start()
{
  playing_.store(true);
  play_thread_ = std::jthread{
    std::bind(&Channel<PubT, TweenT>::run_, this)};
}

template <typename PubT, typename TweenT>
void Channel<PubT, TweenT>::stop()
{
  playing_.store(false);
  if (play_thread_.joinable()) {
    play_thread_.join();
  }
}

template <typename PubT, typename TweenT>
void Channel<PubT, TweenT>::run_()
{
  while (playing_.load()) {
    {
      std::lock_guard lock{tween_mut_};
      // update tween and publish
      if (t_) {
        tween2pub(t_->step(static_cast<int32_t>(rate_.count())));
      }
    }
    // sleep
    std::this_thread::sleep_for(rate_);
  }
}


}  // namespace expression
