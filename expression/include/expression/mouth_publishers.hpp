#pragma once

#include <memory>
#include <string>

#include "expression/grouped_publishers.hpp"

#include "face_msgs/msg/param1_d.hpp"
#include "face_msgs/msg/param2_d.hpp"

namespace expression
{

class MouthPublishers : public GroupedPublishers
{
public:
  using SharedPtr = std::shared_ptr<MouthPublishers>;

  explicit MouthPublishers(
    rclcpp::Node * parent,
    const rclcpp::QoS & qos);

  using Pub1D = rclcpp::Publisher<face_msgs::msg::Param1D>::SharedPtr;

  Pub1D openness;
  Pub1D wideness;
  Pub1D right_lip_corner;
  Pub1D left_lip_corner;
};

}  // namespace expression
