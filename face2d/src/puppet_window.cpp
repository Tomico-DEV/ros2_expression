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
 * \file puppet_window.cpp
 * \brief PuppetWindow class implementation
 *
 * See header for more details
 */

#include "face2d/puppet_window.hpp"

#include <iostream>

namespace face2d
{

std::string get_font_fpath()
{
  using fpath = std::filesystem::path;
  fpath package_path = ament_index_cpp::get_package_share_directory("face2d");
  fpath font_path = package_path / "fonts" / "Tiny5" / "Tiny5-Regular.ttf";

  return font_path.string();
}

PuppetWindow::PuppetWindow(std::string name, sf::Vector2u size, uint32_t style)
: p_window_mutex_{std::make_shared<std::mutex>()},
  window_name_{name}, size_{size}, style_{style}, font_{get_font_fpath()},
  get_params_queue_{}
{ }

PuppetWindow::~PuppetWindow()
{
  stop();
}

void PuppetWindow::start()
{
  bool expected = false;
  if (!running_.compare_exchange_strong(expected, true)) {
    return;  // thread is already running
  }
  window_thread_ = std::thread(&PuppetWindow::update_window_, this);
}

void PuppetWindow::stop()
{
  bool expected = true;
  if (!running_.compare_exchange_strong(expected, false)) {
    return;  // thread is already stopped
  }
  if (window_thread_.joinable()) {
    window_thread_.join();
  }
}

void PuppetWindow::set_close_callback(std::function<void()> callback)
{
  std::lock_guard<std::mutex> lock {*p_window_mutex_};
  close_callback_ = callback;
}

/**
 * \brief render loop
 */
void PuppetWindow::update_window_()
{
  window_.create(sf::VideoMode{size_}, window_name_, style_);
  window_.setVerticalSyncEnabled(true);

  load_puppet_();
  get_puppet_params_();

  Camera cam{};
  cam.set_size(sf::Vector2f{
    static_cast<float>(size_.x),
    static_cast<float>(size_.y)});
  cam.update();

  sf::Vector2i prev_mouse_pos = sf::Mouse::getPosition(window_);

  // graphics elements
  constexpr int tooltips_size = 30;
  sf::Text tooltips{font_};
  tooltips.setString("Ctrl+S: Save config");
  tooltips.setCharacterSize(tooltips_size);
  tooltips.setFillColor(sf::Color(255, 255, 255, 100));
  // anchor text to bottom left
  sf::FloatRect bounds = tooltips.getLocalBounds();
  tooltips.setOrigin(sf::Vector2f{-5.0f, bounds.size.y + tooltips_size * 0.25});

  sf::Clock clock;

  // gl setup
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);


  while (running_.load()) {
    float dt = clock.restart().asSeconds();

    sf::Vector2i cur_mouse_pos = sf::Mouse::getPosition(window_);
    sf::Vector2i delta = cur_mouse_pos - prev_mouse_pos;
    prev_mouse_pos = cur_mouse_pos;

    while (const std::optional event = window_.pollEvent()) {
      // close
      if (event->is<sf::Event::Closed>()) {
        window_.close();
        running_.store(false);
        if (close_callback_) {
          close_callback_();
        }
      }
      // window resize
      if (const auto * resized = event->getIf<sf::Event::Resized>()) {
        size_.x = resized->size.x;
        size_.y = resized->size.y;
        cam.set_size(sf::Vector2f{
          static_cast<float>(size_.x),
          static_cast<float>(size_.y)});
        sf::FloatRect visible_area{{0.f, 0.f}, sf::Vector2f(resized->size)};
        window_.setView(sf::View(visible_area));
        glViewport(0, 0, size_.x, size_.y);
      }
      // wheel scroll
      if (window_.hasFocus()) {
        if (const auto * scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
          cam.set_zoom(
            std::clamp<float>(
                cam.get_zoom() * std::pow(zoom_multiplier, scroll->delta),
                zoom_min,
                zoom_max));
        }
      }
    }

    cam.update();

    // middle click pan
    if (
      window_.hasFocus() &&
      sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
    {
      cam.set_pos(cam.get_pos() + sf::Vector2f{delta} / cam.get_zoom());
    }

    // start draw
    window_.clear(sf::Color::Black);

    // draw puppet (OpenGL)
    {
      std::lock_guard<std::mutex> lock{*p_window_mutex_};
      if (p_puppet_) {
        update_puppet_params_();

        in_puppet_update(p_puppet_, dt);
        in_puppet_draw(p_puppet_, dt);

        if (glGetError() != GL_NO_ERROR) {
          std::cout << "U hab error lmao\n";
        }
      }
    }

    // draw tooltips
    if (window_.hasFocus()) {
      if (
        (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl))
      )
      {
        tooltips.setPosition(sf::Vector2f{0.0f, static_cast<float>(window_.getSize().y)});
        window_.draw(tooltips);
      }
    }

    while (!get_params_queue_.empty()) {
      get_params_queue_.front().set_value(p_puppet_params_);
      get_params_queue_.pop();
    }

    window_.display();
  }

  in_puppet_free(p_puppet_);

  window_.close();
}

/**
 * \brief load the puppet from the filepath.
 * \warning ALWAYS CALL WITHIN GL CONTEXT OR YOU *WILL* GET A SEGFAULT
 */
void PuppetWindow::load_puppet_()
{
  std::lock_guard<std::mutex> lock{*p_window_mutex_};
  if (!p_puppet_) {
    p_puppet_ = in_puppet_load(puppet_filepath_.c_str());
  }
}

/**
 * \brief get puppet parameters.
 * emplaces paras into puppet_params_
 */
void PuppetWindow::get_puppet_params_()
{
  std::lock_guard<std::mutex> lock{*p_window_mutex_};

  p_puppet_params_ = std::make_shared<puppet_params_t>();

  uint32_t count = 0;
  in_parameter_t ** params = in_puppet_get_parameters(p_puppet_, &count);
  size_t param_count = static_cast<size_t>(count);

  for (size_t i = 0; i < param_count; i++) {
    auto param = params[i];
    std::string param_name {in_parameter_get_name(param)};

    if (in_parameter_get_dimensions(param) == 2) {  // is vec2
      Parameter2D param2d {param_name, p_window_mutex_};
      RCLCPP_INFO(
        rclcpp::get_logger("face2d"),
        std::format("Found 2d param: {}", param2d.get_name()).c_str());
      p_puppet_params_->emplace(param2d.get_name(), std::move(param2d));
    } else {  // is vec1
      Parameter1D param1d {param_name, p_window_mutex_};
      RCLCPP_INFO(
        rclcpp::get_logger("face1d"),
        std::format("Found 1d param: {}", param1d.get_name()).c_str());
      p_puppet_params_->emplace(param1d.get_name(), std::move(param1d));
    }
  }
}

/**
 * \brief update puppet parameters in GL context
 * \warning CALL THIS IN THE GL CONTEXT OR SEGFAULT
 */
void PuppetWindow::update_puppet_params_()
{
  if (p_puppet_) {
    uint32_t param_count = 0;
    in_parameter_t ** params = in_puppet_get_parameters(p_puppet_, &param_count);

    for (uint32_t i = 0; i < param_count; i++) {
      auto in_param = params[i];
      auto param_variant = p_puppet_params_->at(std::string{in_parameter_get_name(in_param)});
      
      if (std::holds_alternative<Parameter1D>(param_variant)) {
        Parameter1D param = std::get<Parameter1D>(param_variant);
        if (param.is_changed()) {
          float new_val = param.consume_value();
          in_parameter_set_value(in_param, in_vec2_t{new_val, 0.0f});
        }
      }
      if (std::holds_alternative<Parameter2D>(param_variant)) {
        Parameter2D param = std::get<Parameter2D>(param_variant);
        if (param.is_changed()) {
          sf::Vector2f new_val = param.consume_value();
          in_parameter_set_value(in_param, in_vec2_t{new_val.x, new_val.y});
        }
      }
    }
  }
}

void PuppetWindow::set_puppet(const std::string & fpath)
{
  std::lock_guard<std::mutex> lock{*p_window_mutex_};
  if (std::filesystem::exists(fpath)) {
    puppet_filepath_ = fpath;
  } else {
    throw std::runtime_error("Provided file does not exist!");
  }
}

auto PuppetWindow::get_params() -> std::future<std::shared_ptr<puppet_params_t>>
{
  std::promise<std::shared_ptr<puppet_params_t>> param_promise;
  auto param_future = param_promise.get_future();

  {
    std::lock_guard<std::mutex> lock{*p_window_mutex_};
    get_params_queue_.push(std::move(param_promise));
  }

  return param_future;
}

void PuppetWindow::reload_puppet()
{
  std::lock_guard<std::mutex> lock{*p_window_mutex_};
  if (running_.load()) {
    (void) window_.setActive(true);
    if (p_puppet_) {
      in_puppet_free(p_puppet_);
    }
    p_puppet_ = in_puppet_load(puppet_filepath_.c_str());
  }
}

}  // namespace face2d
