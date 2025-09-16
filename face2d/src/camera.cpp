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
 * \file camera.cpp
 * \brief Camera class implementation
 *
 * See headers for more details
 */

#include "face2d/camera.hpp"

namespace face2d
{

Camera::Camera(sf::Vector2f position, float zoom)
: p_cam_{in_camera_get_current()}, pos_{position}, zoom_{zoom}
{
  in_camera_set_scale(p_cam_, zoom_);
  in_camera_set_position(p_cam_, in_vec2_t{pos_.x, pos_.y});
}

void Camera::set_zoom(float zoom)
{
  zoom_ = zoom;
  in_camera_set_scale(p_cam_, zoom_);
}

auto Camera::get_zoom() -> float
{
  return zoom_;
}

void Camera::update()
{
  in_camera_update(p_cam_);
}

void Camera::set_pos(sf::Vector2f position)
{
  pos_ = position;
  in_camera_set_position(p_cam_, in_vec2_t{pos_.x, pos_.y});
}

auto Camera::get_pos() -> sf::Vector2f
{
  return pos_;
}

void Camera::set_size(sf::Vector2f size)
{
  in_camera_set_size(p_cam_, in_vec2_t{size.x, size.y});
}

auto Camera::get_size() -> sf::Vector2f
{
  auto size = in_camera_get_size(p_cam_);
  return sf::Vector2f{size.x, size.y};
}

}  // namespace face2d
