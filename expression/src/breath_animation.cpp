#include "expression/breath_animation.hpp"

#include <iostream>

namespace expression
{

HasSpeakTask::HasSpeakTask(
  const std::string & name,
  const BT::NodeConfiguration & config,
  std::function<bool()> check_func)
: ConditionNode{name, config}, check_func_{check_func}
{ }

auto HasSpeakTask::tick() -> BT::NodeStatus
{
  if (check_func_()) {
    return BT::NodeStatus::SUCCESS;
  }

  return BT::NodeStatus::FAILURE;
}

Breathe::Breathe(
  const std::string & name,
  const BT::NodeConfiguration & config,
  Channel1D::SharedPtr p_diaphragm_chan,
  std::function<double()> get_bpm,
  std::function<double()> get_setpoint)
: SyncActionNode{name, config}, p_chan_{p_diaphragm_chan},
  get_bpm_{get_bpm}, get_setpoint_{get_setpoint}
{ }


auto Breathe::tick() -> BT::NodeStatus
{
  if (p_chan_->is_running()) {
    // wait for previous tween to finish
    if (p_chan_->done()) {
      // bpm * 1m / 60s => bps invert => spb * 1000 => mspb
      const float duration = (1 / get_bpm_()) * 60 * 1000;

      std::cout << "Setting new setpoint\n";

      if (breath_state_ == BreathState::INHALE) {
        const double inhale_duration =
          std::max(duration * inhale_dur_ratio_, min_inhale_rate_);

        const double setpoint =
          std::clamp<double>(
            neutral_pos_ + get_setpoint_() + setpoint_noise_(),
            0, 1);

        p_chan_->set_tween(
          tweeny::from(p_chan_->get_current())
            .to(setpoint)
            .during(inhale_duration)
            .via(tweeny::easing::cubicInOut));
      } else {  // exhale
        const float exhale_duration =
          std::max(duration * exhale_dur_ratio_, min_exhale_rate_);

        const double setpoint =
          std::clamp<double>(
            neutral_pos_ - get_setpoint_() + setpoint_noise_(),
            0, 1);

        p_chan_->set_tween(
          tweeny::from(p_chan_->get_current())
            .to(setpoint)
            .during(exhale_duration)
            .via(tweeny::easing::quadraticInOut));
      }

      breath_state_ =
        (breath_state_ == BreathState::INHALE)
        ? BreathState::EXHALE
        : BreathState::INHALE;
    }
  }

  return BT::NodeStatus::SUCCESS;
}

double Breathe::setpoint_noise_()
{
  static std::mt19937 engine{std::random_device{}()};
  std::normal_distribution<double> distribution{0, setpoint_stddev_};

  return distribution(engine);
}

BreathAnimator::BreathAnimator(
  std::function<bool()> has_speak_task_func,
  Channel1D::SharedPtr p_diaphragm_chan,
  const std::filesystem::path & tree_file)
{
  generate_bpm_tween_();

  BT::BehaviorTreeFactory factory;

  factory.registerBuilder<HasSpeakTask>(
    "HasSpeakTask",
    [has_speak_task_func](
      const std::string & name,
      const BT::NodeConfiguration & config)
    {
      return std::make_unique<HasSpeakTask>(name, config, has_speak_task_func);
    });

  factory.registerBuilder<Breathe>(
    "Breathe",
    [this, p_diaphragm_chan](
      const std::string & name,
      const BT::NodeConfiguration & config)
    {
      return std::make_unique<Breathe>(
        name, config, p_diaphragm_chan,
        std::bind(&BreathAnimator::get_bpm_, this),
        std::bind(&BreathAnimator::get_target_setpoint_, this));
    });

  tree_ = factory.createTreeFromFile(tree_file.string());
}

void BreathAnimator::generate_bpm_tween_()
{
  static std::mt19937 engine{std::random_device{}()};
  std::normal_distribution<double> distribution{average_bpm_, bpm_stddev_};

  double next_value = distribution(engine);
  double from_value = (bpm_tween_) ?
    bpm_tween_->seek(bpm_tween_->progress())
    : average_bpm_;

  bpm_tween_ =
    tweeny::from(from_value)
    .to(next_value)
    .during(bpm_tween_duration_)
    .via(tweeny::easing::cubicInOut);
}

}  // namespace expression
