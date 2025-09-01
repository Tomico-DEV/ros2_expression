#include "face2d/puppet_window.hpp"

#include <iostream>


namespace face2d
{

PuppetWindow::PuppetWindow(std::string name, sf::Vector2u size, uint32_t style)
: p_window_mutex_ { std::make_shared<std::mutex>() }, 
window_name_ { name }, size_ { size }, style_ { style }
{ }

void PuppetWindow::start()
{
    running_.store(true);
    window_thread_ = std::thread(&PuppetWindow::update_window_, this);
}

void PuppetWindow::stop()
{
    running_.store(false);
    if (window_thread_.joinable())
    {
        window_thread_.join();
    }
}

void PuppetWindow::set_close_callback(std::function<void()> callback)
{
    std::lock_guard<std::mutex> lock { *p_window_mutex_ };
    close_callback_ = callback;
}

/**
 * \brief render loop
 */
void PuppetWindow::update_window_()
{
    window_.create(sf::VideoMode { size_ }, window_name_, style_); 
    window_.setVerticalSyncEnabled(true);

    // initialize inochi2d
    inInit([]() -> double { 
        return static_cast<double>(sf::Clock().getElapsedTime().asSeconds()); 
    });

    load_puppet_();
    get_puppet_params_();

    inViewportSet(size_.x, size_.y);
    Camera cam { };

    sf::Vector2i prev_mouse_pos = sf::Mouse::getPosition(window_);

    while (running_.load())
    {
        sf::Vector2i cur_mouse_pos = sf::Mouse::getPosition(window_);
        sf::Vector2i delta = cur_mouse_pos - prev_mouse_pos;
        prev_mouse_pos = cur_mouse_pos;

        while (const std::optional event = window_.pollEvent())
        {
            // close
            if (event->is<sf::Event::Closed>())
            {
                window_.close();
                running_.store(false);
                if (close_callback_)
                    close_callback_();
            }
            // window resize
            if (const auto * resized = event->getIf<sf::Event::Resized>())
            {
                size_.x = resized->size.x;
                size_.y = resized->size.y;
                glViewport(0, 0, size_.x, size_.y);
                inViewportSet(size_.x, size_.y);
            }
            // wheel scroll
            if (const auto * scroll = event->getIf<sf::Event::MouseWheelScrolled>())
            {
                cam.set_zoom(
                    std::clamp<float>(
                        cam.get_zoom() * std::pow(zoom_multiplier, scroll->delta),
                        zoom_min,
                        zoom_max
                    )
                );
            }
            
        }
        // middle click pan
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
        {
            cam.set_pos(cam.get_pos() + sf::Vector2f { delta } / cam.get_zoom());
        }
        
        (void) window_.setActive(true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //float elapsed = clock.getElapsedTime().asSeconds();
        //std::get<Parameter1D>(puppet_params_.at("Mouth Openness")).set_value(std::sin(elapsed * 2));
        
        {
            std::lock_guard<std::mutex> lock { *p_window_mutex_ };
            if (p_puppet_)
            {
                update_puppet_params_();
                inUpdate();
                inSceneBegin();
                inPuppetUpdate(p_puppet_);
                inPuppetDraw(p_puppet_);
                inSceneEnd();
                inSceneDraw(0, 0, size_.x, size_.y);
            }
        }

        while (!get_params_queue_.empty())
        {
            get_puppet_params_();
            get_params_queue_.front().set_value(p_puppet_params_);
            get_params_queue_.pop();
        }

        window_.display();
    }

    inPuppetDestroy(p_puppet_);
    // unload inochi2d runtime
    inCleanup();
}

/**
 * \brief load the puppet from the filepath.
 * \warning ALWAYS CALL WITHIN GL CONTEXT OR YOU *WILL* GET A SEGFAULT
 */
void PuppetWindow::load_puppet_()
{
    std::lock_guard<std::mutex> lock { *p_window_mutex_ };
    std::cout << puppet_filepath_ << "\n";
    if (!p_puppet_)
        p_puppet_ = inPuppetLoad(puppet_filepath_.c_str());
}

/**
 * \brief get puppet parameters.
 * emplaces paras into puppet_params_
 */
void PuppetWindow::get_puppet_params_()
{
    std::lock_guard<std::mutex> lock { *p_window_mutex_ };

    p_puppet_params_ =  std::make_shared<puppet_params_t>(); 
    
    InParameter ** params = nullptr;
    size_t param_count = 0;

    inPuppetGetParameters(p_puppet_, &params, &param_count);

    for (size_t i = 0; i < param_count; i++)
    {
        auto param = params[i];
        std::string param_name { inParameterGetName(param) };
        if (inParameterIsVec2(param))
        {
            Parameter2D param2d { param_name, p_window_mutex_ };
            std::cout << "Found 2d param: " << param2d.get_name() << "\n";
            p_puppet_params_->emplace(param2d.get_name(), std::move(param2d));
        } 
        else
        {
            Parameter1D param1d { param_name, p_window_mutex_ };
            std::cout << "Found 1d param: " << param1d.get_name() << "\n";
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
    InParameter ** params = nullptr;
    size_t param_count = 0;

    inPuppetGetParameters(p_puppet_, &params, &param_count);

    for (size_t i = 0; i < param_count; i++)
    {
        auto in_param = params[i];
        auto param_variant = p_puppet_params_->at(std::string { inParameterGetName(in_param) });

        if (std::holds_alternative<Parameter1D>(param_variant))
        {
            Parameter1D param = std::get<Parameter1D>(param_variant);
            if (param.is_changed())
            {
                float new_val = param.consume_value();
                std::cout << "Update " << param.get_name() << " to " << new_val << "\n";
                inParameterSetValue(in_param, new_val, 0.0f);
            }
        } 
        if (std::holds_alternative<Parameter2D>(param_variant))
        {
            Parameter2D param = std::get<Parameter2D>(param_variant);
            if (param.is_changed())
            {
                sf::Vector2f new_val = param.consume_value();
                std::cout << "Update " << param.get_name() << " to " << new_val.x << ", " << new_val.y << "\n";
                inParameterSetValue(in_param, new_val.x, new_val.y);
            
            }
        }

        inParameterDestroy(in_param);
    }
}

void PuppetWindow::set_puppet(const std::string& fpath)
{
    std::lock_guard<std::mutex> lock { *p_window_mutex_ };
    if (std::filesystem::exists(fpath))
        puppet_filepath_ = fpath;
    else
        throw std::runtime_error("Provided file does not exist!");
}

auto PuppetWindow::get_params() -> std::future<std::shared_ptr<puppet_params_t>>
{
    std::promise<std::shared_ptr<puppet_params_t>> param_promise;
    auto param_future = param_promise.get_future();
    
    {
        std::lock_guard<std::mutex> lock { *p_window_mutex_ };
        get_params_queue_.push(std::move(param_promise));
    }

    return param_future;
}

void PuppetWindow::reload_puppet()
{
    std::lock_guard<std::mutex> lock { *p_window_mutex_ };
    if (running_.load())
    {
        (void) window_.setActive(true);
        if (p_puppet_)
            inPuppetDestroy(p_puppet_);
        p_puppet_ = inPuppetLoad(puppet_filepath_.c_str());
    }
    
}

}
