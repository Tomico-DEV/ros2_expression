#include "simple_blink_animator/blink_animator.hpp"


namespace expression
{

HasStareEvent::HasStareEvent(
  const std::string & name,
  const BT::NodeConfiguration & config,
  std::function<bool()> check_func)
: ConditionNode{name, config}, check_func_{check_func}
{ }

auto HasStareEvent::tick() -> BT::NodeStatus
{
  if (check_func_()) {
    return BT::NodeStatus::SUCCESS;
  }

  return BT::NodeStatus::FAILURE;
}

Blink::Blink(
  const std::string & name,
  const BT::NodeConfiguration & config,
  ChannelCore1D::SharedPtr p_left_eyelid,
  ChannelCore1D::SharedPtr p_right_eyelid)
: SyncActionNode{name, config},
  p_left_eyelid_{p_left_eyelid}, p_right_eyelid_{p_right_eyelid},
  engine_{std::random_device{}()}
{ }


auto Blink::tick() -> BT::NodeStatus
{
  if (p_left_eyelid_ && p_right_eyelid_) {
    if (
      p_left_eyelid_->is_running() &&
      p_right_eyelid_->is_running())
    {
      // wait for previous blink to finish
      if (p_left_eyelid_->done() && p_right_eyelid_->done()) {
        const double base_interval = generate_interval_();
        const double offset = interval_noise_();
        const double left_blink_dur = generate_blink_dur_();
        const double right_blink_dur = generate_blink_dur_();

        tweeny::tween left_tween = tweeny::from<double>(0.0);
        tweeny::tween right_tween = tweeny::from<double>(0.0);

        double right_interval = base_interval;
        double left_interval = base_interval;

        // apply a bit of offset for unevenness
        if (offset > 0) {  // right delay
          right_interval -= offset;
          right_tween = add_interval_(right_tween, offset);
        } else {  // left delay
          left_interval += offset;
          left_tween = add_interval_(left_tween, -offset);
        }

        left_tween = add_blink_(left_tween, left_blink_dur);
        right_tween = add_blink_(right_tween, right_blink_dur);
        left_tween = add_interval_(left_tween, left_interval);
        right_tween = add_interval_(right_tween, right_interval);

        p_left_eyelid_->set_tween(left_tween);
        p_right_eyelid_->set_tween(right_tween);
      }
    }
  }

  return BT::NodeStatus::SUCCESS;
}

/**
 * \brief generate blink interval
 */
double Blink::generate_interval_()
{
  std::normal_distribution<double>
    distribution{avg_interval_, interval_stddev_};

  return
    std::clamp<double>(distribution(engine_),
      min_interval_, max_interval_);
}

/**
 * \brief generate blink duration
 */
double Blink::generate_blink_dur_()
{
  std::normal_distribution<double>
    distribution{avg_blink_duration_, blink_duration_stddev_};

  return distribution(engine_);
}

double Blink::interval_noise_()
{
  std::normal_distribution<double>
    distribution{0, noise_stddev_};

  return distribution(engine_);
}

/**
 * \brief generate start blink interval noise
 */

auto Blink::add_blink_(
  tweeny::tween<double> tween,
  double blink_dur)
-> tweeny::tween<double>
{
  const double stride_duration = blink_dur * blink_stride_ratio_;
  const double closed_duration = blink_dur * blink_close_ratio_;
  const double open_duration = blink_dur - stride_duration - closed_duration;

  return tween
    .to(1).during(stride_duration).via(tweeny::easing::quinticIn)
    .to(1).during(closed_duration)
    .to(0).during(open_duration).via(tweeny::easing::sinusoidalOut);
}

auto Blink::add_interval_(
  tweeny::tween<double> tween,
  double interval)
-> tweeny::tween<double>
{
  return tween.
    to(0).during(interval);
}

void BlinkAnimator::initialize(std::shared_ptr<ChanMap> p_chan_map, const ParamMap&)
{
  using fpath = std::filesystem::path;

  fpath package_path = ament_index_cpp::get_package_share_directory("simple_blink_animator");
  fpath tree_path = package_path / "behavior_trees" / "blink_anim.xml";

  BT::BehaviorTreeFactory factory;

  factory.registerBuilder<HasStareEvent>(
    "HasStareEvent",
    [this](
      const std::string & name,
      const BT::NodeConfiguration & config)
    {
      return std::make_unique<HasStareEvent>(
        name, config,
        std::bind(&BlinkAnimator::has_stare_event_, this));
    });

  ChannelCore1D::SharedPtr p_left_eyelid = get_1d_chan_(*p_chan_map, "left_eye/eyelid");
  ChannelCore1D::SharedPtr p_right_eyelid = get_1d_chan_(*p_chan_map, "right_eye/eyelid");

  factory.registerBuilder<Blink>(
    "Blink",
    [this, p_left_eyelid, p_right_eyelid](
      const std::string & name,
      const BT::NodeConfiguration & config)
    {
      return std::make_unique<Blink>(
        name, config,
        p_left_eyelid, p_right_eyelid);
    });

  tree_ = factory.createTreeFromFile(tree_path.string());
}


}  // namespace expression

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(expression::BlinkAnimator, expression::Animator)
