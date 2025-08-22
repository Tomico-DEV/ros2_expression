#pragma once

#include <utility>
#include <string>
#include <mutex>

#include <SFML/System.hpp>

#define INOCHI2D_GLYES
#include <inochi2d.h>

namespace face2d
{

template <typename T>
class ParameterBase
{
public:
    ParameterBase(InParameter * param, std::shared_ptr<std::mutex> p_parent_mutex);
    ~ParameterBase();

    std::string get_name() const;

    virtual void set_value(const T& val) = 0;
    virtual T get_value() = 0;
    virtual void update_value() = 0;
protected:
    InParameter * param_;
    std::string name_;
    std::shared_ptr<std::mutex> p_parent_mutex_;
};

class Parameter1D : public ParameterBase<float>
{
public:
    inline Parameter1D(InParameter * param, std::shared_ptr<std::mutex> p_parent_mutex)
    : ParameterBase<float>(param, p_parent_mutex)
    { }
    
    void set_value(const float& val) override;
    float get_value() override;
    void update_value() override;
private:
    float val_ = 0.0f;
};

class Parameter2D : public ParameterBase<sf::Vector2f>
{
public:
    inline Parameter2D(InParameter * param, std::shared_ptr<std::mutex> p_parent_mutex)
    : ParameterBase<sf::Vector2f>(param, p_parent_mutex)
    { }
    
    void set_value(const sf::Vector2f& val) override;
    sf::Vector2f get_value() override;
    void update_value() override;
private:
    sf::Vector2f val_ { 0.0f, 0.0f };
};

template <typename T>
ParameterBase<T>::ParameterBase(InParameter * param, std::shared_ptr<std::mutex> p_parent_mutex)
: param_ { param }, name_{ inParameterGetName(param) }, p_parent_mutex_ { p_parent_mutex }
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
