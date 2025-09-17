#pragma once

#include <any>
#include <string>

namespace expression
{

struct Event
{
  enum class Type
  {
    SPEAK,
    GAZE
  };

  inline Event(Type type, std::any data)
  : type{type}, data{data}
  { }

  Type type;
  std::any data;
};

struct Vec3D
{
  double x;
  double y;
  double z;
};

template<Event::Type>
struct EventDataType;

template<>
struct EventDataType<Event::Type::SPEAK> {
  using type = std::string;
};

template<>
struct EventDataType<Event::Type::GAZE> {
  using type = Vec3D;
};

template<Event::Type T>
using EventDataType_t = typename EventDataType<T>::type;

}  // namespace expression
