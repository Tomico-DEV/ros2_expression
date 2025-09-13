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
 * \file puppet_window.hpp
 * \brief Window manager and rendering class
 */

#pragma once

#define INOCHI2D_GLYES
#include <inochi2d.h>

#include <atomic>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

// ROS2
#include "rclcpp/logging.hpp"
#include "rclcpp/clock.hpp"

// SFML
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

// face2d
#include <face2d/camera.hpp>
#include <face2d/parameter.hpp>


namespace face2d
{

typedef std::unordered_map<std::string, std::variant<Parameter1D, Parameter2D>> puppet_params_t;

/**
 * \brief pupper renderer and window class
 */
class PuppetWindow
{
public:
  /**
   * \brief Initializer
   * \param name name of the window
   * \param size size of the window
   * \param style style of the window. Defaults to sf::Style::Default
   * Doesn't actually create a sfml window. To actually open a window,
   * PuppetWindow::start() should be invoked
   */
  PuppetWindow(std::string name, sf::Vector2u size, uint32_t style = sf::Style::Default);
  ~PuppetWindow();
  /**
   * \brief opens and starts the rendering thread
   */
  void start();
  /**
   * \brief closes the window and stops the rendering thread
   */
  void stop();
  /**
   * \brief sets puppet
   * \param p_puppet shared pointer to the Puppet object
   */
  void set_puppet(const std::string & fpath);
  /**
   * \brief get puppet parameters
   */
  std::future<std::shared_ptr<puppet_params_t>> get_params();
  /**
   * \brief reloads puppet given that an existing puppet exists
   */
  void reload_puppet();
  /**
   * \brief set callback for when the window closes
   */
  void set_close_callback(std::function<void()> callback);

  // ui
  std::atomic<float> zoom_multiplier = 1.1f;
  std::atomic<float> zoom_min = 0.01f;
  std::atomic<float> zoom_max = 4.0f;

private:
  void update_window_();
  void load_puppet_();
  void get_puppet_params_();
  void update_puppet_params_();

  // inochi2d
  std::string puppet_filepath_;
  InPuppet * p_puppet_ = nullptr;
  std::shared_ptr<puppet_params_t> p_puppet_params_;
  std::queue<std::promise<std::shared_ptr<puppet_params_t>>> get_params_queue_;

  // thread and sfml
  std::atomic<bool> running_{false};
  std::thread window_thread_;
  std::shared_ptr<std::mutex> p_window_mutex_;
  sf::Window window_;
  std::string window_name_;
  sf::Vector2u size_;
  uint32_t style_;

  // callbacks
  std::function<void()> close_callback_ = nullptr;
};

}  // namespace face2d
