#pragma once

#include <format>
#include <string>
#include <unordered_map>

#include "rclcpp/rclcpp.hpp"

namespace expression
{

/**
 * \brief GroupedPublishers base class
 * 
 * Helper class for keeping publishers organized since 
 * the face has tons of DOFs and therefore subscribers. 
 */
class GroupedPublishers
{
public:
  explicit inline GroupedPublishers(
    rclcpp::Node * parent,
    const std::string & root,
    const rclcpp::QoS & qos)
  : parent_{parent}, root_{root}, qos_{qos}
  { }

protected:
  /**
   * \brief convenience function for parent->create_publisher
   */
  template <typename T>
  auto make_pub_(const std::string & topic)
  -> rclcpp::Publisher<T>::SharedPtr
  {
    return parent_->create_publisher<T>(
    std::format("{}/{}", root_, topic), qos_);
  }

private:
  rclcpp::Node * parent_;
  std::string root_;
  rclcpp::QoS qos_;
};

}  // namespace expression
