#pragma once

#include <thread>
#include <utility>
#include <string>
#include <mutex>

#include <SFML/System.hpp>

#define INOCHI2D_GLYES
#include <inochi2d.h>

namespace face2d
{

/**
 * \brief Parameter class for representing puppet parameters
 * 
 * Doesn't actually store the raw pointers to the params. That is handled by
 * PuppetWindow in its update loop because of lifetime issues with inParameter
 * (ask me how I know..)
 */
template <typename T>
class ParameterBase
{
public:
    /**
     * \param name Name of parameter
     * \param p_parent_mutex Shared pointer to the window's mutex
     */
    ParameterBase(std::string name, std::shared_ptr<std::mutex> p_parent_mutex);

    /**
     * \brief Get name of parameter
     */
    std::string get_name() const;

    /**
     * \brief Set value. Locks parent's mutex
     */
    virtual void set_value(const T& val) = 0;
    
    /**
     * \brief Get value. Locks parent's mutex
     */
    T get_value();
    /**
     * \brief Get value and reset the changed flag
     * \warning Does not lock parent mutex. Always call within context!
     */
    T consume_value();
    /**
     * \brief Returns true if parameter has been changed, but has not been reflected
     * \warning Does not lock parent mutex. Always call within context!
     */
    bool is_changed();

protected:
    virtual T get_value_nolock() = 0;

    std::string name_;
    std::shared_ptr<std::mutex> p_parent_mutex_;
    bool changed_ = false; // true when updates have not been reflected 
};

class Parameter1D : public ParameterBase<float>
{
public:
    inline Parameter1D(std::string name, std::shared_ptr<std::mutex> p_parent_mutex)
    : ParameterBase<float>(name, p_parent_mutex)
    { }
    Parameter1D& operator=(const Parameter1D&) = default;
    
    void set_value(const float& val) override;
private:
    float get_value_nolock() override;

    float val_ = 0.0f;
};

class Parameter2D : public ParameterBase<sf::Vector2f>
{
public:
    inline Parameter2D(std::string name, std::shared_ptr<std::mutex> p_parent_mutex)
    : ParameterBase<sf::Vector2f>(name, p_parent_mutex)
    { }
    Parameter2D& operator=(const Parameter2D&) = default;

    void set_value(const sf::Vector2f& val) override;

private:
    sf::Vector2f get_value_nolock() override;

    sf::Vector2f val_ { 0.0f, 0.0f };
};

// template defs

template <typename T>
ParameterBase<T>::ParameterBase(std::string name, std::shared_ptr<std::mutex> p_parent_mutex)
: name_{ name }, p_parent_mutex_ { p_parent_mutex }
{ }


template <typename T>
auto ParameterBase<T>::get_name() const -> std::string
{
    return name_;
}

template <typename T>
bool ParameterBase<T>::is_changed()
{
    return changed_;
}

template <typename T>
T ParameterBase<T>::get_value()
{
    std::lock_guard<std::mutex> lock { *p_parent_mutex_ };
    return get_value_nolock();
}

template <typename T>
T ParameterBase<T>::consume_value()
{
    changed_ = false;
    return get_value_nolock();
}

}
