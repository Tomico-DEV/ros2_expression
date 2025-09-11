#pragma once

#include <memory>
#include <string>

#include "expression/grouped_publishers.hpp"
#include "expression/utils.hpp"

#include "face_msgs/msg/param1_d.hpp"
#include "face_msgs/msg/param2_d.hpp"


namespace expression
{

class EyePublishers : public GroupedPublishers
{
public:
  using SharedPtr = std::shared_ptr<EyePublishers>;

  explicit EyePublishers(
    rclcpp::Node * parent,
    const std::string & root,
    const rclcpp::QoS & qos);

  Pub2D eye;
  Pub1D brow_inner, brow_outer;
  Pub1D eyelid;
  Pub1D cheek;
};

}  // namespace expression
