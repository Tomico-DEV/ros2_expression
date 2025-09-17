#pragma once

#include <atomic>
#include <utility>
#include <memory>
#include <mutex>
#include <optional>

#include "tweeny/tweeny.h"

namespace expression
{

struct Vec2D
{
  double x, y;

  inline Vec2D operator+(const Vec2D& o) const { return {x + o.x, y + o.y}; }
  inline Vec2D operator-(const Vec2D& o) const { return {x - o.x, y - o.y}; }
  inline Vec2D operator*(double d) const { return {x * d, y * d}; }
  inline Vec2D operator-() const { return {-x, -y}; }
  inline Vec2D operator/(double d) const { return {x / d, y / d}; }
};

/**
 * @brief Channel without the ROS2 impl
 */
template <typename TweenT>
class ChannelCore
{
public:
  using SharedPtr = std::shared_ptr<ChannelCore<TweenT>>;

  virtual ~ChannelCore()
  {
    std::lock_guard lock{tween_mut_};
    t_.reset();
  }

  inline void set_tween(tweeny::tween<TweenT> tween)
  {
    std::lock_guard lock{tween_mut_};
    t_ = std::move(tween);
  }
  inline TweenT get_current()
  {
    std::lock_guard lock{tween_mut_};
    if (t_) {
      return t_->seek(t_->progress());
    }
    return TweenT{};  // default construct
  }
  inline bool done()
  {
    std::lock_guard lock{tween_mut_};
    if (!t_.has_value()) {
      return true;
    }

    return t_->progress() >= 1.0f;
  }

  virtual void start() = 0;
  virtual void stop() = 0;
  virtual bool is_running() = 0;

protected:
  std::optional<tweeny::tween<TweenT>> t_;
  std::mutex tween_mut_;
};

using ChannelCore1D = ChannelCore<double>;
using ChannelCore2D = ChannelCore<Vec2D>;

}  // namespace expression
