#pragma once

#include <string>
#include <map>

#include "face_msgs/msg/param1_d.hpp"
#include "face_msgs/msg/param2_d.hpp"

#include "expression/channel_core.hpp"

#include "rclcpp/rclcpp.hpp"

namespace expression
{

using Param1D = face_msgs::msg::Param1D;
using Param2D = face_msgs::msg::Param2D;

using Pub1D = rclcpp::Publisher<Param1D>::SharedPtr;
using Pub2D = rclcpp::Publisher<Param2D>::SharedPtr;

using ChanMap =
  std::map<
    std::string,
    std::variant<ChannelCore1D::SharedPtr, ChannelCore2D::SharedPtr>
  >;

}  // namespace expression
