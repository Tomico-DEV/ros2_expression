#pragma once

#include <filesystem>
#include <map>
#include <future>
#include <memory>
#include <string>
#include <cstddef>

#include "rclcpp/rclcpp.hpp"
#include "ament_index_cpp/get_package_share_directory.hpp"

#include "face2d/puppet_window.hpp"
#include "face_msgs/msg/param1_d.hpp"
#include "face_msgs/msg/param2_d.hpp"

#include "yaml-cpp/yaml.h"

namespace face2d
{

class Face2DNode : public rclcpp::Node
{
public:
    Face2DNode();
    ~Face2DNode();
private:
    void declare_params_();
    void make_subscribers_();
    void flatten_mappings_(
        const YAML::Node& node,
        std::map<std::string, std::string>& out_map,
        const std::string& current_path
    );

    sf::Vector2u get_window_size_();
    uint32_t get_window_style_();
    std::unique_ptr<PuppetWindow> p_window_;
    
    std::vector<
        rclcpp::Subscription<face_msgs::msg::Param1D
    >::SharedPtr> subscriber_1d_;
    std::vector<
        rclcpp::Subscription<face_msgs::msg::Param2D
    >::SharedPtr> subscriber_2d_;

    // std::unordered_map<std::string, std::variant<Parameter1D, Parameter2D>> puppet_params_;
};

}