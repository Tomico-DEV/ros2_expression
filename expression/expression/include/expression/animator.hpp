#pragma once

#include <chrono>
#include <format>
#include <string>
#include <map>
#include <memory>

#include "expression/channel_core.hpp"
#include "expression/event.hpp"

namespace expression
{

class Animator
{
public:
  using SharedPtr = std::shared_ptr<Animator>;
  using ChanMap =
    std::map<
      std::string,
      std::variant<ChannelCore1D::SharedPtr, ChannelCore2D::SharedPtr>
    >;

  virtual ~Animator() = default;
  virtual void initialize(std::shared_ptr<ChanMap> p_chan_map) = 0;
  /**
   * @brief get animator name
   */
  virtual auto get_name() -> std::string = 0;
  /**
   * @brief handle event
   */
  virtual void handle_event(const Event& e) = 0;
  /**
   * @brief update animation
   */
  virtual void update(std::chrono::milliseconds dt) = 0;
protected:
  static inline auto get_1d_chan_(
    ChanMap chan_p_map, const std::string & chan_name)
  -> ChannelCore1D::SharedPtr
  {
    ChannelCore1D::SharedPtr p_chan;

    try {
      auto maybe_channel = chan_p_map.at(chan_name);
      if (std::holds_alternative<ChannelCore1D::SharedPtr>(maybe_channel)) {
        p_chan = std::get<ChannelCore1D::SharedPtr>(maybe_channel);
      } else {
        throw std::runtime_error{
          std::format("{} channel is not 1D!", chan_name)};
      }
    } catch (const std::exception & e) {
      throw std::runtime_error{
        std::format("Error getting channel {}: {}", chan_name, e.what())};
    }

    return p_chan;
  }
};

}  // namespace expression
