#pragma once

#include <memory>
#include <string>

#include "expression/grouped_channels.hpp"


namespace expression
{

class MouthChannels : public GroupedChannels
{
public:
  using SharedPtr = std::shared_ptr<MouthChannels>;

  explicit MouthChannels(
   rclcpp::Node * parent,
    const rclcpp::QoS & qos,
    std::shared_ptr<ChanMap> p_chan_map,
    std::chrono::milliseconds rate);
};

}  // namespace expression
