#include "expression/eye_publishers.hpp"

namespace expression
{

EyePublishers::EyePublishers(
  rclcpp::Node * parent,
  const std::string & root,
  const rclcpp::QoS & qos)
: GroupedPublishers{parent, root, qos},
  eye{make_pub_<Param2D>("eye")},
  brow_inner{make_pub_<Param1D>("brow_inner")},
  brow_outer{make_pub_<Param1D>("brow_outer")},
  eyelid{make_pub_<Param1D>("eyelid")},
  cheek{make_pub_<Param1D>("cheek")}
{ }

}  // namespace expression
