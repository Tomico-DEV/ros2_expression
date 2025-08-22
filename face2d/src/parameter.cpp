#include "face2d/parameter.hpp"

#include <iostream>

namespace face2d
{

void Parameter1D::set_value(const float& val)
{
    std::lock_guard<std::mutex> lock { *p_parent_mutex_ };
    val_ = val;
}

auto Parameter1D::get_value() -> float
{
    std::lock_guard<std::mutex> lock { *p_parent_mutex_ };
    float x, y;
    inParameterGetValue(param_, &x, &y);
    return x;
}

void Parameter1D::update_value()
{
    std::cout << "Update 1D parameter\n";
    inParameterSetValue(param_, val_, 0.0f);
}

void Parameter2D::set_value(const sf::Vector2f& val)
{
    std::lock_guard<std::mutex> lock { *p_parent_mutex_ };
    val_ = val;
}

auto Parameter2D::get_value() -> sf::Vector2f
{
    std::lock_guard<std::mutex> lock { *p_parent_mutex_ };
    float x, y;
    inParameterGetValue(param_, &x, &y);
    return sf::Vector2f { x, y };
}

void Parameter2D::update_value()
{
    std::cout << "Update 2D parameter\n";
    inParameterSetValue(param_, val_.x, val_.y);
}

}