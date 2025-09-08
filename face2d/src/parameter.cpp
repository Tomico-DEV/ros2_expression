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
 * \file parameter.cpp
 * \brief Paramter classes implementation
 *
 * See headers for more detail
 */

#include "face2d/parameter.hpp"


namespace face2d
{

void Parameter1D::set_value(const float & val)
{
  std::lock_guard<std::mutex> lock{*p_parent_mutex_};
  val_ = val;
  changed_ = true;
}

auto Parameter1D::get_value_nolock() -> float
{
  return val_;
}

void Parameter2D::set_value(const sf::Vector2f & val)
{
  std::lock_guard<std::mutex> lock{*p_parent_mutex_};
  val_ = val;
  changed_ = true;
}

auto Parameter2D::get_value_nolock() -> sf::Vector2f
{
  return val_;
}

}  // namespace face2d
