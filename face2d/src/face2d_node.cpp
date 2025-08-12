#include "face2d/face2d_node.hpp"

#ifndef INOCHI2D_GLYES
#define INOCHI2D_GLYES
#endif
#include <inochi2d.h>

namespace face2d
{

Face2DNode::Face2DNode()
: Node { "face2d" }
{
    declare_params_();

    p_window_ = std::make_unique<PuppetWindow>(
        get_parameter("window.title").as_string(), 
        get_window_size_(),
        get_window_style_()
    );
    p_window_->set_puppet(get_parameter("puppet_file").as_string());
    p_window_->start();
}

Face2DNode::~Face2DNode()
{
    p_window_->stop();
}

auto Face2DNode::get_window_size_() -> sf::Vector2u
{
    // input check
    int width = get_parameter("window.width").as_int();
    int height = get_parameter("window.height").as_int();

    if (width < 1 or height < 1)
        throw std::runtime_error("Window size must be > 0!");

    return sf::Vector2u { static_cast<uint>(width), static_cast<uint>(height) };
}

auto Face2DNode::get_window_style_() -> uint32_t
{
    int style = get_parameter("window.style").as_int();

    if (style < 0)
        throw std::runtime_error("Style must be a uint32!");
    
    return static_cast<uint32_t>(style);
}

void Face2DNode::declare_params_()
{
    typedef rcl_interfaces::msg::ParameterDescriptor ParamDesc;
    typedef rcl_interfaces::msg::IntegerRange IntRange;

    IntRange positive_range;
    positive_range.from_value = 0;
    positive_range.to_value = std::numeric_limits<int64_t>::max();
    positive_range.step = 1;

    auto make_desc = [&](const std::string &desc_text, bool limited=false) {
        ParamDesc desc;
        if (limited)
            desc.integer_range.push_back(positive_range);
        desc.description = desc_text;
        return desc;
    };

    declare_parameter<int>("window.width", 800, make_desc("Width of window (pixels)", true));
    declare_parameter<int>("window.height", 600, make_desc("Height of window (pixels)", true));
    declare_parameter<std::string>("window.title", "Face2D", make_desc("Title of the window"));
    declare_parameter<int>(
        "window.style",
        static_cast<int>(sf::Style::Default),
        make_desc("SFML style of window (sf::style). See SFML docs for more info", true)
    );
    
    // puppet
    std::filesystem::path package_path = ament_index_cpp::get_package_share_directory("face2d");
    std::filesystem::path default_puppet_path = package_path / "puppet" / "maru_face.inx";
    declare_parameter<std::string>(
        "puppet_file",
        default_puppet_path.string(),
        [&]{
            ParamDesc desc;
            desc.description = "Filepath to puppet file";
            return desc;
        }()
    );

}

}