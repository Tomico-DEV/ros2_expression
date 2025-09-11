#include "expression/neck_publishers.hpp"


namespace expression
{

NeckPublishers::NeckPublishers(
  rclcpp::Node * parent,
  const rclcpp::QoS & qos)
: GroupedPublishers{parent, "neck", qos},
  pitch_yaw{make_pub_<Param2D>("pitch_yaw")},
  roll{make_pub_<Param1D>("roll")}
{ }

}  // namespace expression
