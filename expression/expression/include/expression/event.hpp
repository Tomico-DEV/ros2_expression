#pragma once
#include <any>

namespace expression
{

struct Event
{
  enum class Type
  {
    SPEAK
  };

  Type type;
  std::any data;
};

}  // namespace expression
