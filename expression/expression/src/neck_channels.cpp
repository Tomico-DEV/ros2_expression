#include "expression/neck_channels.hpp"


namespace expression
{

NeckChannels::NeckChannels(
  rclcpp_lifecycle::LifecycleNode * parent,
  const rclcpp::QoS & qos,
  std::shared_ptr<ChanMap> p_chan_map,
  std::chrono::milliseconds rate)
: GroupedChannels{parent, "neck", qos, p_chan_map, rate}
{
  make_chan_<Channel2D>("pitch_yaw");
  make_chan_<Channel1D>("roll");

  // add diaphragm to
  std::string name = "diaphragm";
  Channel1D::SharedPtr p_diaphragm =
    std::make_shared<Channel1D>(
      parent->create_publisher<Param1D>(name, qos));

  (*p_chan_map)[name] = p_diaphragm;
  chans_.push_back(p_diaphragm);
}

}  // namespace expression
