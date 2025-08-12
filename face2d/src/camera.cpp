#include "face2d/camera.hpp"

namespace face2d
{

Camera::Camera(sf::Vector2f position, float zoom)
: p_cam_ { inCameraGetCurrent() }, pos_ { position }, zoom_ { zoom }
{
    inCameraSetZoom(p_cam_, zoom_);
    inCameraSetPosition(p_cam_, pos_.x, pos_.y);
}

void Camera::set_zoom(float zoom)
{
    zoom_ = zoom;
    inCameraSetZoom(p_cam_, zoom_);
}

auto Camera::get_zoom() -> float
{
    return zoom_;
}

void Camera::set_pos(sf::Vector2f position)
{
    pos_ = position;
    inCameraSetPosition(p_cam_, pos_.x, pos_.y);
}

auto Camera::get_pos() -> sf::Vector2f
{
    return pos_;
}

}