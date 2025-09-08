// Copyright 2025 TomicoDEV
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 *                    -
 *   /\                 \       _________________
 *  //\\                 \     /                 \
 * //  \\          <<<    |   |  ROS2 EXPRESSION  |
 *             <<<<        |   \ ________________/
 *          <<             |   |/ 
 *             <<<<        |
 *                 <<<    |
 *                       /
 *                      /
 *                    -
 * \author TomicoDEV
 * \file speaker_node.cpp
 * \brief Speaker Node implementation
 *
 * See header for details
 */

#include "voicevox_speaker/speaker_node.hpp"

namespace voicevox
{

/**
 * \brief convenience function for turning string vecs into strings
 */
auto vec2str(const std::vector<std::string> & vec) -> std::string
{
  std::string res;
  for (const auto & elem : vec) {
    res += elem;
    if (elem != vec.back()) {
      res += ", ";
    }
  }

  return res;
}

/**
 * \brief convenience function for tunring shapeset into stringvecs
 * \param s input ShapeSet
 * \return vector of converted shapes
 */
auto shapeset2strvec(const ShapeSet & s) -> std::vector<std::string>
{
  std::vector<std::string> strvec;
  std::transform(
    s.begin(), s.end(),
    std::back_inserter(strvec),
    [&](const Shape & shape) {
      return std::format("{}", shape);  // convert to string
    });

  return strvec;
}

VVSpeakerNode::VVSpeakerNode(const rclcpp::NodeOptions & options)
: Node{"voicevox_speaker", options}
{
  declare_params_();
  init_action_server_();

  p_voicevox_ = std::make_shared<Voicevox>(
    dict_path_, ort_path_, model_paths_,
    voicevox_make_default_initialize_options());
}

/**
 * \brief declare ros2 parameters
 */
void VVSpeakerNode::declare_params_()
{
  // typedefs to keep things pretty
  typedef rcl_interfaces::msg::ParameterDescriptor ParamDesc;
  typedef std::filesystem::path fpath;
  // convenience function for returning ParamDesc objects
  auto make_desc =
    [&](const std::string & desc_text) {
      ParamDesc desc;
      desc.description = desc_text;
      return desc;
    };

  // Voicevox runtime

  // default paths
  fpath package_path = ament_index_cpp::get_package_share_directory("voicevox_speaker");
  fpath dict_path = package_path / "dict" / "open_jtalk_dic_utf_8-1.11";
  fpath ort_path = package_path / "onnx_lib" /
    voicevox_get_onnxruntime_lib_versioned_filename();
  fpath model_path = package_path / "models" / "vvms" / "0.vvm";

  // declare path params
  dict_path_ = declare_parameter<std::string>(
    "voicevox.dictionary_path", dict_path.string(),
    make_desc("Path to OpenJtalk dictionary"));
  ort_path_ = declare_parameter<std::string>(
    "voicevox.onnxruntime_lib_path", ort_path.string(),
    make_desc("Path to onnxruntime library"));
  auto model_pathstrings = declare_parameter<std::vector<std::string>>(
    "voicevox.model_path", {model_path.string()},
    make_desc("Paths to voicemodel files to load"));

  // convert strings to filepaths
  model_paths_.reserve(model_pathstrings.size());
  std::transform(
    model_pathstrings.begin(), model_pathstrings.end(),
    std::back_inserter(model_paths_),
    [](const std::string & s) {
      return fpath{s};
    });

  // Rhubarb

  // generate extended mouthshapes string vector for default use
  ShapeSet basic_shapes{ShapeConverter::get().getBasicShapes()};
  ShapeSet extended_shapes{basic_shapes};
  extended_shapes.insert({Shape::G, Shape::H, Shape::X});
  std::vector<std::string> basic_shapes_str =
    shapeset2strvec(basic_shapes);
  std::vector<std::string> extended_shapes_str =
    shapeset2strvec(extended_shapes);

  // declare mouth shape param
  std::vector<std::string> mouth_shape_strs =
    declare_parameter<std::vector<std::string>>(
      "rhubarb.mouth_shapes", extended_shapes_str,
      make_desc("A list of mouth shapes available to use"));
  // parse mouth shape param to Rhubarb's ShapeSet
  try {
    std::transform(
      mouth_shape_strs.begin(), mouth_shape_strs.end(),
      std::inserter(mouth_shapes_, mouth_shapes_.end()),
      [](const std::string & s) {
        std::istringstream is { s };
        Shape shape;
        is >> shape;
        return shape;
      });
  } catch (const std::exception& e) {
    throw std::runtime_error(
      std::format(
        "There was an error parsing the provided mouthshapes: {} (error: {})",
        vec2str(mouth_shape_strs), e.what()));
  }
  // Check if basic shapes are present

  if (
    !std::includes(
      mouth_shapes_.begin(), mouth_shapes_.end(),
      basic_shapes.begin(), basic_shapes.end()))
  {
    // If basic shapes are missing, throw (TO DO)
    throw std::runtime_error(
      std::format(
        "Provided mouthshapes ({}) doesn't include basic mouthshapes ({})",
        vec2str(mouth_shape_strs), vec2str(basic_shapes_str)));
  }

  // Node params

  update_rate_ = declare_parameter<double>(
    "feedback_update_rate", update_rate_,  // default rate is 100
    make_desc("How often to send back speak action viseme feedback"));
}

void VVSpeakerNode::init_action_server_()
{
  action_server_ = rclcpp_action::create_server<SpeakAction>(
    this, "speak",
    std::bind(
      &VVSpeakerNode::handle_speak_goal_, this, std::placeholders::_1,
      std::placeholders::_2),
    std::bind(
      &VVSpeakerNode::handle_speak_cancel_, this,
      std::placeholders::_1),
    std::bind(
      &VVSpeakerNode::handle_accepted_, this, std::placeholders::_1));
}

/**
 * \brief Tries to generate AudioQuery from goal request
 *        and accepts if no errors occur
 */
auto VVSpeakerNode::handle_speak_goal_(
  const rclcpp_action::GoalUUID &uuid,
  std::shared_ptr<const SpeakAction::Goal> goal)
-> rclcpp_action::GoalResponse
{
  RCLCPP_INFO(get_logger(), "Got goal: %s", goal->text.c_str());

  try {
    auto new_query = std::make_shared<AudioQuery>(
      p_voicevox_->make_audio_query(goal->text, speaker_id_));

    // Stop previous playback if running
    if (running_.load()) {
      stop_playback_();
    }

    // Replace the current query with the new one
    p_query_ = std::move(new_query);
  } catch (const std::exception & e) {
    RCLCPP_WARN(get_logger(), "Failed to make audio query!: %s", e.what());
    return rclcpp_action::GoalResponse::REJECT;
  }

  (void)uuid;

  return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

/**
 * \brief Cancels playback
 */
auto VVSpeakerNode::handle_speak_cancel_(
  const std::shared_ptr<GoalHandleSpeak> goal_handle)
-> rclcpp_action::CancelResponse
{
  RCLCPP_INFO(get_logger(), "Cancelling goal..");
  (void)goal_handle;
  running_.store(false);

  return rclcpp_action::CancelResponse::ACCEPT;
}

/**
 * \brief Starts playback thread
 */
void VVSpeakerNode::handle_accepted_(
  const std::shared_ptr<GoalHandleSpeak> goal_handle)
{
  // needs to return quickly so we don't block the executor
  playback_thread_ =
    std::jthread{&VVSpeakerNode::playback_, this, goal_handle, update_rate_};
}

/**
 * \brief playback (synthesize) query and viseme
 */
void VVSpeakerNode::playback_(
  const std::shared_ptr<GoalHandleSpeak> goal_handle, double update_rate)
{
  running_.store(true);
  // create synthesis stream
  SynthesisStream synth_stream_{*p_query_, speaker_id_, p_voicevox_};

  // parse audioquery to phonemes
  BoundedTimeline<Phone> phone_timeline = p_query_->to_timeline();
  // parse phoneme timeline to viseme timeline
  JoiningContinuousTimeline<Shape> viseme_timeline =
    animate(phone_timeline, mouth_shapes_);

  auto update_dt = std::chrono::milliseconds(std::lround(1000.0 / update_rate));

  auto start_time = std::chrono::steady_clock::now();
  centiseconds elapsed_time{0};  // elapsed time
  centiseconds end_time =
    viseme_timeline.getRange().getEnd();  // end time in centiseconds

  // default viseme at start
  Shape viseme{Shape::X};

  double volume = p_query_->get_volume();

  // start audio playback and viseme playback at the same time
  synth_stream_.play();

  while (elapsed_time < end_time && running_.load()) {
    if (goal_handle->is_canceling()) {
      goal_handle->canceled(std::make_shared<SpeakAction::Result>());
      running_.store(false);
      synth_stream_.cancel();
      return;
    }

    auto now = std::chrono::steady_clock::now();
    elapsed_time = std::chrono::round<centiseconds>(now - start_time);

    if (elapsed_time > end_time) {
      break;
    }

    // sample from timeline
    auto opt = viseme_timeline.get(elapsed_time);
    viseme = opt ? opt->getValue() : Shape::X;

    SpeakAction::Feedback feedback;
    feedback.viseme = std::format("{}", viseme);  // parse to str
    feedback.volume = volume;
    goal_handle->publish_feedback(
        std::make_shared<SpeakAction::Feedback>(feedback));

    std::this_thread::sleep_for(update_dt);
  }

  // wait for cancel or for the playback to finish
  while (
    running_.load() &&
    synth_stream_.getStatus() == sf::SoundSource::Status::Playing)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  running_.store(false);
  RCLCPP_INFO(get_logger(), "Playback complete");

  if (rclcpp::ok()) {
    auto result = std::make_shared<SpeakAction::Result>();
    result->success = true;
    goal_handle->succeed(result);
    RCLCPP_INFO(get_logger(), "Goal succeeded");
  }
}

void VVSpeakerNode::stop_playback_() {
  running_.store(false);

  if (playback_thread_.joinable()) {
    playback_thread_.join();
  }

  playback_thread_ = {};  // reset jthread
  p_query_.reset();       // destroy current AudioQuery
}

}  // namespace voicevox

RCLCPP_COMPONENTS_REGISTER_NODE(voicevox::VVSpeakerNode)
