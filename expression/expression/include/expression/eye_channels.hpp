#pragma once

#include <memory>
#include <string>

#include "expression/grouped_channels.hpp"
#include "expression/utils.hpp"


namespace expression
{

class EyeChannels : public GroupedChannels
{
public:
  using SharedPtr = std::shared_ptr<EyeChannels>;

  EyeChannels(
    rclcpp::Node * parent,
    const std::string & root,
    const rclcpp::QoS & qos,
    std::shared_ptr<ChanMap> p_chan_map,
    std::chrono::milliseconds rate);
};

}  // namespace expression
