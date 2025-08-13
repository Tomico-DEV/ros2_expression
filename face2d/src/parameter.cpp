#include "face2d/parameter.hpp"

namespace face2d
{

void Parameter1D::set_value(const float& val)
{
    inParameterSetValue(param_, val, 0);
}

auto Parameter1D::get_value() -> float
{
    float x, y;
    inParameterGetValue(param_, &x, &y);
    return x;
}

void Parameter2D::set_value(const sf::Vector2f& val)
{
    inParameterSetValue(param_, val.x, val.y);
}

auto Parameter2D::get_value() -> sf::Vector2f
{
    float x, y;
    inParameterGetValue(param_, &x, &y);
    return sf::Vector2f { x, y };
}

}