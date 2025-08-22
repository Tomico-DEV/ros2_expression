#include "face2d/parameter.hpp"

#include <iostream>

namespace face2d
{

void Parameter1D::set_value(const float& val)
{
    std::lock_guard<std::mutex> lock { *p_parent_mutex_ };
    val_ = val;
    changed_ = true;
}

auto Parameter1D::get_value_nolock() -> float
{
    return val_;
}


void Parameter2D::set_value(const sf::Vector2f& val)
{
    std::lock_guard<std::mutex> lock { *p_parent_mutex_ };
    val_ = val;
    changed_ = true;
}

auto Parameter2D::get_value_nolock() -> sf::Vector2f
{
    return val_;
}

}