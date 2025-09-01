#include "voicevox_speaker/speaker_node.hpp"

namespace voicevox
{

VVSpeakerNode::VVSpeakerNode(const rclcpp::NodeOptions & options)
: Node { "voicevox_speaker", options } /* sound_ { buffer_ } */
{
    declare_params_();
    init_action_server_();

    p_voicevox_ = std::make_shared<Voicevox>(
        dict_path_,
        ort_path_,
        model_paths_,
        voicevox_make_default_initialize_options()
    );

}

void VVSpeakerNode::declare_params_()
{
    typedef rcl_interfaces::msg::ParameterDescriptor ParamDesc;
    
    auto make_desc = [&](const std::string &desc_text) {
        ParamDesc desc;
        desc.description = desc_text;
        return desc;
    };

    // default paths
    std::filesystem::path package_path = ament_index_cpp::get_package_share_directory("voicevox_speaker");
    std::filesystem::path dict_path = package_path / "dict" / "open_jtalk_dic_utf_8-1.11";
    std::filesystem::path ort_path = package_path / "onnx_lib" / voicevox_get_onnxruntime_lib_versioned_filename();
    std::filesystem::path model_path = package_path / "models" / "vvms" / "0.vvm";
        
    dict_path_ = declare_parameter<std::string>(
        "voicevox.dictionary_path",
        dict_path.string(),
        make_desc("Path to OpenJtalk dictionary")
    );
    ort_path_ = declare_parameter<std::string>(
        "voicevox.onnxruntime_lib_path",
        ort_path.string(),
        make_desc("Path to onnxruntime library")
    );
    auto model_pathstrings = declare_parameter<std::vector<std::string>>(
        "voicevox.model_path_",
        { model_path.string() },
        make_desc("Paths to voicemodel files to load")
    );
    // convert strings to filepaths
    model_paths_.reserve(model_pathstrings.size());
    std::transform(
        model_pathstrings.begin(),
        model_pathstrings.end(),
        std::back_inserter(model_paths_),
        [](const std::string& s) { return std::filesystem::path { s }; }
    );
}

void VVSpeakerNode::init_action_server_()
{
    action_server_ = rclcpp_action::create_server<SpeakAction>(
        this,
        "speak",
        std::bind(&VVSpeakerNode::handle_speak_goal_, this, 
            std::placeholders::_1, std::placeholders::_2),
        std::bind(&VVSpeakerNode::handle_speak_cancel_, this,
            std::placeholders::_1),
        std::bind(&VVSpeakerNode::handle_accepted_, this,
            std::placeholders::_1)
    );
}

auto VVSpeakerNode::handle_speak_goal_(
    const rclcpp_action::GoalUUID & uuid,
    std::shared_ptr<const SpeakAction::Goal> goal
) -> rclcpp_action::GoalResponse
{
    RCLCPP_INFO(get_logger(), "Got goal: %s", goal->text.c_str());
    
    try
    {
        p_query_ = std::make_shared<AudioQuery>(
            p_voicevox_->make_audio_query(goal->text, speaker_id_)
        );
    }
    catch (const std::exception& e)
    {
        RCLCPP_WARN(get_logger(), "Failed to make audio query!: %s", e.what());
        return rclcpp_action::GoalResponse::REJECT;
    }
    (void)uuid;

    // parse ok -- override current sound
    if (running_.load())
        stop_playback_();
    
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;  
}

auto VVSpeakerNode::handle_speak_cancel_(
    const std::shared_ptr<GoalHandleSpeak> goal_handle
) -> rclcpp_action::CancelResponse
{
    RCLCPP_INFO(get_logger(), "Cancelling goal..");
    (void)goal_handle;
    running_.store(false);

    return rclcpp_action::CancelResponse::ACCEPT;
}

void VVSpeakerNode::handle_accepted_(
    const std::shared_ptr<GoalHandleSpeak> goal_handle
)
{
    // needs to return quickly so we don't block the executor
    playback_thread_ = std::jthread {
        &VVSpeakerNode::playback_,
        this,
        goal_handle
    };
}

void VVSpeakerNode::playback_(
    const std::shared_ptr<GoalHandleSpeak> goal_handle
)
{
    running_.store(true);
    // create synthesis stream
    SynthesisStream synth_stream_ {
        *p_query_, speaker_id_, p_voicevox_
    };
    // parse audioquery to visemes
    BoundedTimeline<Phone> phone_timeline = p_query_->to_timeline();
    ShapeSet shapes { ShapeConverter::get().getBasicShapes() };
    // add extra shapes
    shapes.insert(Shape::G);
    shapes.insert(Shape::H);
    shapes.insert(Shape::X);
    JoiningContinuousTimeline<Shape> viseme_timeline = 
        animate(phone_timeline, shapes);
    
    const int update_rate = 100;
    const auto update_dt = std::chrono::milliseconds(1000 / update_rate);

    auto start_time = std::chrono::steady_clock::now();
    centiseconds elapsed_time { 0 }; // elapsed time
    centiseconds end_time = viseme_timeline.getRange().getEnd(); // end time in centiseconds

    Shape viseme { Shape::X };
    
    // start audio playback and viseme playback at the same time
    synth_stream_.play();

    while (elapsed_time < end_time)
    {
        if (goal_handle->is_canceling())
        {
            goal_handle->canceled(std::make_shared<SpeakAction::Result>());
            running_.store(false);
            synth_stream_.cancel();
            return;
        }

        auto now = std::chrono::steady_clock::now();
        elapsed_time = std::chrono::round<centiseconds>(now - start_time);

        if (elapsed_time > end_time)
            break;
        
        // sample from timeline
        auto opt = viseme_timeline.get(elapsed_time);
        viseme = opt ? opt->getValue() : Shape::X;

        SpeakAction::Feedback feedback;
        std::ostringstream oss;
        oss << viseme;
        feedback.viseme = oss.str();
        feedback.duration.sec = 0; // placeholder for now, will impl properly later
        feedback.duration.nanosec = 0;
        feedback.volume = 1.0;
        goal_handle->publish_feedback(std::make_shared<SpeakAction::Feedback>(feedback));

        std::this_thread::sleep_for(update_dt);
    }

    
    // wait for cancel or for the playback to finish
    while (running_.load() && synth_stream_.getStatus() == sf::SoundSource::Status::Playing)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }

    synth_stream_.cancel();
    running_.store(false);
    
    if (rclcpp::ok()) {
        auto result = std::make_shared<SpeakAction::Result>();
        result->success = true;
        goal_handle->succeed(result);
        RCLCPP_INFO(this->get_logger(), "Goal succeeded");
    }
}


}

RCLCPP_COMPONENTS_REGISTER_NODE(voicevox::VVSpeakerNode)
