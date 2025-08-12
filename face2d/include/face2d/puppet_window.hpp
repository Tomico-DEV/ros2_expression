#pragma once

#include <atomic>
#include <algorithm>
#include <chrono>
#include <string>
#include <thread>
#include <cmath>
#include <memory>
#include <mutex>
#include <vector>
#include <optional>
#include <filesystem>

#include "rclcpp/logging.hpp"
#include "rclcpp/clock.hpp"

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#define INOCHI2D_GLYES
#include <inochi2d.h>

#include <face2d/camera.hpp>


namespace face2d
{


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
    PuppetWindow(std::string name, sf::Vector2u size, uint32_t style=sf::Style::Default);

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
    void set_puppet(const std::string& fpath);

    /**
     * \brief reloads puppet given that an existing puppet exists
     */
    void reload_puppet();

    void set_close_callback(std::function<void()> callback);
    
    // ui
    std::atomic<float> zoom_multiplier = 1.1f;
    std::atomic<float> zoom_min = 0.01f;
    std::atomic<float> zoom_max = 4.0f;
private:
    void update_window_();
    // inochi2d
    std::string puppet_filepath_;
    InPuppet * p_puppet_ = nullptr;
    
    // thread and sfml
    std::atomic<bool> running_ = false;
    std::thread window_thread_;
    std::mutex window_mutex_;
    sf::Window window_;
    std::string window_name_;
    sf::Vector2u size_;
    uint32_t style_;

    // callbacks
    std::function<void()> close_callback_ = nullptr;

    // logging
    /*
    rclcpp::Logger logger_;
    rclcpp::Clock::SharedPtr p_clock_;
    */
};

} 