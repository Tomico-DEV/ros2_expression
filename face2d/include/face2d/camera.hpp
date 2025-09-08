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
 * \file camera.hpp
 * \brief C++ wrapper for Inochi2D's camera
 *
 * See header for details
 */

#pragma once

#define INOCHI2D_GLYES
#include <inochi2d.h>

#include <SFML/System.hpp>

namespace face2d
{

/**
 * \warning ALWAYS DEFINE IN AN INOCHI2D CONTEXT, OTHERWISE YOU WILL GET
 *          A SEGFAULT. Ask me how I found out...
 */
class Camera
{
public:
  /**
   * \param position starting position of camera
   * \param zoom starting zoom of camera
   */
  explicit Camera(sf::Vector2f position = sf::Vector2{0.0f, 0.0f}, float zoom = 0.8f);

  void set_zoom(float zoom);
  float get_zoom();

  void set_pos(sf::Vector2f position);
  sf::Vector2f get_pos();

private:
  InCamera * p_cam_ = nullptr;
  sf::Vector2f pos_;
  float zoom_;
};

}  // namespace face2d
