#include "expression/eye_channels.hpp"

namespace expression
{

EyeChannels::EyeChannels(
  rclcpp_lifecycle::LifecycleNode * parent,
  const std::string & root,
  const rclcpp::QoS & qos,
  std::shared_ptr<ChanMap> p_chan_map,
  std::chrono::milliseconds rate)
: GroupedChannels{parent, root, qos, p_chan_map, rate}
{
  make_chan_<Channel2D>("eye");
  make_chan_<Channel1D>("brow_inner");
  make_chan_<Channel1D>("brow_outer");
  make_chan_<Channel1D>("eyelid");
  make_chan_<Channel1D>("cheek");
}

}  // namespace expression
