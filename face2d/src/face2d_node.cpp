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
 * \file face2d_node.cpp
 * \brief Face2d Node class implementation
 *
 * See headers for more details
 */

#define INOCHI2D_GLYES
#include <inochi2d.h>

#include <iostream>

#include "face2d/face2d_node.hpp"


namespace face2d
{

Face2DNode::Face2DNode()
: Node{"face2d"}
{
  declare_params_();

  p_window_ = std::make_unique<PuppetWindow>(
    get_parameter("window.title").as_string(),
    get_window_size_(),
    get_window_style_());
  p_window_->set_puppet(get_parameter("puppet_file").as_string());
  p_window_->start();

  make_subscribers_();

  RCLCPP_INFO(get_logger(), "Got params!");
}

Face2DNode::~Face2DNode()
{
  p_window_->stop();
}

/**
 * \brief get window size specified from param
 * \return size
 */
auto Face2DNode::get_window_size_() -> sf::Vector2u
{
  // input check
  int width = get_parameter("window.width").as_int();
  int height = get_parameter("window.height").as_int();

  if (width < 1 || height < 1) {
    throw std::runtime_error("Window size must be > 0!");
  }

  return sf::Vector2u{static_cast<uint>(width), static_cast<uint>(height)};
}

/**
 * \brief get window style specified from param
 * \return uint32 style
 */
auto Face2DNode::get_window_style_() -> uint32_t
{
  int style = get_parameter("window.style").as_int();

  if (style < 0) {
    throw std::runtime_error("Style must be a uint32!");
  }

  return static_cast<uint32_t>(style);
}

/**
 * \brief declare ros2 parameters
 */
void Face2DNode::declare_params_()
{
  typedef rcl_interfaces::msg::ParameterDescriptor ParamDesc;
  typedef rcl_interfaces::msg::IntegerRange IntRange;

  IntRange positive_range;
  positive_range.from_value = 0;
  positive_range.to_value = std::numeric_limits<int64_t>::max();
  positive_range.step = 1;

  // convenience function for making parameter description
  auto make_desc =
    [&](const std::string & desc_text, bool limited = false)
    {
      ParamDesc desc;
      if (limited) {
        desc.integer_range.push_back(positive_range);
      }
      desc.description = desc_text;
      return desc;
    };

  declare_parameter<int>("window.width", 800, make_desc("Width of window (pixels)", true));
  declare_parameter<int>("window.height", 600, make_desc("Height of window (pixels)", true));
  declare_parameter<std::string>("window.title", "Face2D", make_desc("Title of the window"));
  declare_parameter<int>(
    "window.style",
    static_cast<int>(sf::Style::Default),
    make_desc("SFML style of window (sf::style). See SFML docs for more info", true));

  // puppet parameters
  std::filesystem::path package_path = ament_index_cpp::get_package_share_directory("face2d");
  std::filesystem::path default_puppet_path = package_path / "puppet" / "maru_face.inx";
  declare_parameter<std::string>(
    "puppet_file",
    default_puppet_path.string(),
    make_desc("Filepath to puppet file"));
  std::filesystem::path default_config_path = package_path / "puppet" / "puppet_config.yaml";
  declare_parameter<std::string>(
    "puppet_config_file",
    default_config_path.string(),
    make_desc("Filepath to puppet's config file"));
}

/**
 * \brief convert nested mappings specified in puppet's yaml into
 *        a map of string paths
 * \param [in] node root node to search from
 * \param [out] out_map output map to be populated
 * \param [in] current_path string path of provided root node
 */
void Face2DNode::flatten_mappings_(
  const YAML::Node & node,
  std::map<std::string, std::string> & out_map,
  const std::string & current_path
)
{
  if (!node.IsMap()) {
    return;
  }

  // node is a map - get key and valueuint32 style
  for (auto & entry : node) {
    std::string key = entry.first.as<std::string>();
    const YAML::Node & value = entry.second;

    // append key to current path
    std::string new_path = current_path + "/" + key;

    if (value.IsMap()) {
      flatten_mappings_(value, out_map, new_path);  // keep going..
    } else if (value.IsScalar()) {
      out_map[new_path] = value.as<std::string>();  // reached end, add key and val to map!
    } else {
      throw std::runtime_error(
        "Unsupported value in param_mappings! Must be map or scalar");
    }
  }
}

/**
 * \brief get puppet params and create corresponding subscribers
 */
void Face2DNode::make_subscribers_()
{
  auto params_future = p_window_->get_params();
  // wait for window to load params
  auto puppet_params = params_future.get();

  YAML::Node param_mapping = YAML::LoadFile(
    get_parameter("puppet_config_file").as_string())["param_mapping"];
  std::map<std::string, std::string> ros2_inochi_param_map;
  flatten_mappings_(param_mapping, ros2_inochi_param_map, "");

  // iterate through param_mapping and create subscribers
  for (auto & [topic, param_name] : ros2_inochi_param_map) {
    auto & param = puppet_params->at(param_name);
    if (std::holds_alternative<Parameter1D>(param)) {
      subscriber_1d_.push_back(
        create_subscription<face_msgs::msg::Param1D>(
        topic, 10,
        [puppet_params, param_name](const face_msgs::msg::Param1D::SharedPtr msg)
        {
          std::get<Parameter1D>(puppet_params->at(param_name)).set_value(msg->val);
        }));
    } else if (std::holds_alternative<Parameter2D>(param)) {
      subscriber_2d_.push_back(
          create_subscription<face_msgs::msg::Param2D>(
          topic, 10,
          [puppet_params, param_name](const face_msgs::msg::Param2D::SharedPtr msg)
          {
            std::get<Parameter2D>(puppet_params->at(param_name)).set_value(
              sf::Vector2f{static_cast<float>(msg->x), static_cast<float>(msg->y)});
          }));
    }
  }

  RCLCPP_INFO(get_logger(), "Subscribers successfully initialized!");
}

}  // namespace face2d
