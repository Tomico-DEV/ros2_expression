#pragma once

#include <atomic>
#include <chrono>
#include <format>
#include <string>
#include <thread>
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"

#include "face_msgs/msg/param1_d.hpp"
#include "face_msgs/msg/param2_d.hpp"

#include "expression/utils.hpp"
#include "expression/channel.hpp"


namespace expression
{

/**
 * \brief GroupedPublishers base class
 * 
 * Helper class for keeping publishers and channels organized 
 * since the face has tons of DOFs and therefore subscribers. 
 */
class GroupedChannels
{
public:
  using SharedPtr = std::shared_ptr<GroupedChannels>;

  explicit inline GroupedChannels(
    rclcpp_lifecycle::LifecycleNode * parent,
    const std::string & root,
    const rclcpp::QoS & qos,
    std::shared_ptr<ChanMap> p_chan_map,
    std::chrono::milliseconds rate)
  : parent_lc_{parent}, p_chan_map_{p_chan_map},
    root_{root}, qos_{qos}, rate_{rate}
  { }

  void start();
  void stop();

protected:
  template <typename T>
  void make_chan_(const std::string & topic);

  std::vector<
    std::variant<Channel1D::SharedPtr, Channel2D::SharedPtr>
  > chans_;

private:
  /**
   * \brief convenience function for parent_->create_publisher
   */
  template <typename T>
  inline auto make_pub_(const std::string & topic_name)
  -> rclcpp::Publisher<T>::SharedPtr
  {
    return parent_lc_->create_publisher<T>(topic_name, qos_);
  }

  void run_();


  rclcpp_lifecycle::LifecycleNode * parent_lc_;
  std::shared_ptr<ChanMap> p_chan_map_;
  std::string root_;
  rclcpp::QoS qos_;

  std::atomic_bool running_{false};
  std::jthread thread_;
  std::chrono::milliseconds rate_;
};

}  // namespace expression
