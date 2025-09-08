// Copyright 2025 TomicoDEV
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 *                    -
 *   /\                 \       _________________
 *  //\\                 \     /                 \
 * //  \\          <<<    |   |  ROS2 EXPRESSION  |
 *             <<<<        |   \ ________________/
 *          <<             |   |/
 *             <<<<        |
 *                 <<<    |
 *                       /
 *                      /
 *                    -
 * \author TomicoDEV
 * \file face2d_node.hpp
 * \brief Dynamic virtual 2D face using Inochi2D
 * 
 * Params:
 *   (TO DO)
 * Topics:
 *   (TO DO why am I so lazy lmao)
 */

#pragma once

#include <cstddef>
#include <filesystem>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <vector>

// YAML
#include "yaml-cpp/yaml.h"

// ROS2
#include "rclcpp/rclcpp.hpp"
#include "ament_index_cpp/get_package_share_directory.hpp"

#include "face_msgs/msg/param1_d.hpp"
#include "face_msgs/msg/param2_d.hpp"

// face2d
#include "face2d/puppet_window.hpp"

namespace face2d
{

/**
 * \brief Face2D Node.
 */
class Face2DNode : public rclcpp::Node
{
public:
  Face2DNode();
  ~Face2DNode();

private:
  void declare_params_();
  void make_subscribers_();
  void flatten_mappings_(
    const YAML::Node & node,
    std::map<std::string, std::string> & out_map,
    const std::string & current_path);

  sf::Vector2u get_window_size_();
  uint32_t get_window_style_();
  std::unique_ptr<PuppetWindow> p_window_;

  std::vector<
    rclcpp::Subscription<face_msgs::msg::Param1D
    >::SharedPtr> subscriber_1d_;
  std::vector<
    rclcpp::Subscription<face_msgs::msg::Param2D
    >::SharedPtr> subscriber_2d_;
};

}  //  namespace face2d
