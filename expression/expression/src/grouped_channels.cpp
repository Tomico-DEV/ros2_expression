#include "expression/grouped_channels.hpp"

#include <iostream>

namespace expression
{

// overload trick
template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

void GroupedChannels::start()
{
  bool expected = false;
  if (!running_.compare_exchange_strong(expected, true)) {
    return;  // thread is already running
  }

  // start individual channels
  for (auto & p_channel : chans_) {
    std::visit(overloaded{
      [&](const Channel1D::SharedPtr & ch) { ch->start(); },
      [&](const Channel2D::SharedPtr & ch) { ch->start(); }
    }, p_channel);
  }

  thread_ = std::jthread{std::bind(&GroupedChannels::run_, this)};
}

void GroupedChannels::stop()
{
  bool expected = true;
  if (!running_.compare_exchange_strong(expected, false)) {
    return;  // thread is already stopped
  }

  if (thread_.joinable()) {
    thread_.join();
  }

  for (auto & ch_variant : chans_) {
    std::visit(overloaded{
      [&](auto & ch) {
        ch->stop();
      }
    }, ch_variant);
  }
}

void GroupedChannels::run_()
{
  while (running_.load()) {
    for (auto & p_channel : chans_) {
      std::visit(overloaded{
        [&](const Channel1D::SharedPtr & ch) { ch->step_(rate_); },
        [&](const Channel2D::SharedPtr & ch) { ch->step_(rate_); }
      }, p_channel);
    }
    std::this_thread::sleep_for(rate_);
  }
}

template void GroupedChannels::make_chan_<Channel1D>(const std::string & topic);
template void GroupedChannels::make_chan_<Channel2D>(const std::string & topic);

template <typename U>
struct ParamType;

template <>
struct ParamType<Channel1D>
{
  using type = Param1D;
};

template <>
struct ParamType<Channel2D>
{
  using type = Param2D;
};

template <typename T>
void GroupedChannels::make_chan_(const std::string & topic)
{
  using P = typename ParamType<T>::type;

  std::string name = std::format("{}/{}", root_, topic);

  std::shared_ptr<T> p_chan = std::make_shared<T>(make_pub_<P>(name));
  (*p_chan_map_)[name] = (p_chan);
  chans_.push_back(p_chan);
}

}  // namespace expression
