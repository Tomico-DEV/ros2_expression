#pragma once

#include <memory>
#include <string>

#include "expression/grouped_publishers.hpp"
#include "expression/utils.hpp"

#include "face_msgs/msg/param1_d.hpp"
#include "face_msgs/msg/param2_d.hpp"

namespace expression
{

class NeckPublishers : public GroupedPublishers
{
public:
  using SharedPtr = std::shared_ptr<NeckPublishers>;

  explicit NeckPublishers(
    rclcpp::Node * parent,
    const rclcpp::QoS & qos);

  Pub2D pitch_yaw;
  Pub1D roll;
};

}  // namespace expression
