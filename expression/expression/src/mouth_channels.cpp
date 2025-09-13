#include "expression/mouth_channels.hpp"


namespace expression
{

using Param1D = face_msgs::msg::Param1D;

MouthChannels::MouthChannels(
  rclcpp_lifecycle::LifecycleNode * parent,
  const rclcpp::QoS & qos,
  std::shared_ptr<ChanMap> p_chan_map,
  std::chrono::milliseconds rate)
: GroupedChannels{parent, "mouth", qos, p_chan_map, rate}
{
  make_chan_<Channel1D>("openness");
  make_chan_<Channel1D>("wideness");
  make_chan_<Channel1D>("right_lip_corner");
  make_chan_<Channel1D>("left_lip_corner");
}

}  // namespace expression
