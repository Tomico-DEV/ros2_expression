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
: p_cam_{inCameraGetCurrent()}, pos_{position}, zoom_{zoom}
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

}  // namespace face2d
