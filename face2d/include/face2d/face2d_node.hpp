#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <cstddef>

#include "rclcpp/rclcpp.hpp"

#include "face2d/puppet_window.hpp"

#include "ament_index_cpp/get_package_share_directory.hpp"

namespace face2d
{

class Face2DNode : public rclcpp::Node
{
public:
    Face2DNode();
    ~Face2DNode();
private:
    void declare_params_();

    sf::Vector2u get_window_size_();
    uint32_t get_window_style_();
    std::unique_ptr<PuppetWindow> p_window_;
};

}