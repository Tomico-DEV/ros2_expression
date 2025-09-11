#include "expression/mouth_publishers.hpp"


namespace expression
{

using Param1D = face_msgs::msg::Param1D;

MouthPublishers::MouthPublishers(
  rclcpp::Node * parent,
  const rclcpp::QoS & qos)
: GroupedPublishers{parent, "mouth", qos},
  openness{make_pub_<Param1D>("openness")},
  wideness{make_pub_<Param1D>("wideness")},
  right_lip_corner{make_pub_<Param1D>("right_lip_corner")},
  left_lip_corner{make_pub_<Param1D>("left_lip_corner")}
{ }

}  // namespace expression
