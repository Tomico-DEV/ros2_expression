#pragma once

#include <memory>
#include <string>

#include "expression/grouped_channels.hpp"
#include "expression/utils.hpp"

namespace expression
{

class NeckChannels : public GroupedChannels
{
public:
  using SharedPtr = std::shared_ptr<NeckChannels>;

  NeckChannels(
    rclcpp::Node * parent,
    const rclcpp::QoS & qos,
    std::shared_ptr<ChanMap> p_chan_map,
    std::chrono::milliseconds rate);
};

}  // namespace expression
