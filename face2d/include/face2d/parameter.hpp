#pragma once

#include <utility>
#include <string>

#include <SFML/System.hpp>

#define INOCHI2D_GLYES
#include <inochi2d.h>

namespace face2d
{

template <typename T>
class ParameterBase
{
public:
    ParameterBase(InParameter * param);
    ~ParameterBase();

    std::string get_name() const;

    virtual void set_value(const T& val) = 0;
    virtual T get_value() = 0;
protected:
    InParameter * param_;
    std::string name_;
};

class Parameter1D : public ParameterBase<float>
{
public:
    inline Parameter1D(InParameter * param)
    : ParameterBase<float>(param)
    { }
    
    void set_value(const float& val) override;
    float get_value() override;
};

class Parameter2D : public ParameterBase<sf::Vector2f>
{
public:
    inline Parameter2D(InParameter * param)
    : ParameterBase<sf::Vector2f>(param)
    { }
    
    void set_value(const sf::Vector2f& val) override;
    sf::Vector2f get_value() override;
};

template <typename T>
ParameterBase<T>::ParameterBase(InParameter * param)
: param_ { param }, name_{ inParameterGetName(param) }
{ }

template <typename T>
ParameterBase<T>::~ParameterBase()
{
    inParameterDestroy(param_);
}

template <typename T>
auto ParameterBase<T>::get_name() const -> std::string
{
    return name_;
}


}
