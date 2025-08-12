#pragma once

#include <SFML/System.hpp>

#define INOCHI2D_GLYES
#include <inochi2d.h>

namespace face2d
{

/**
 * \warning ALWAYS DEFINE IN AN INOCHI2D CONTEXT, OTHERWISE YOU WILL GET
 *          A SEGFAULT. Ask me how I found out...
 */
class Camera 
{
public:
    Camera(sf::Vector2f position=sf::Vector2{ 0.0f, 0.0f }, float zoom=0.8f);

    void set_zoom(float zoom);
    float get_zoom();

    void set_pos(sf::Vector2f position);
    sf::Vector2f get_pos();
private:
    InCamera * p_cam_ = nullptr;
    sf::Vector2f pos_;
    float zoom_;
};

}