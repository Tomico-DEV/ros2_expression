#include "rclcpp/rclcpp.hpp"

#include "voicevox_speaker/speaker_node.hpp"


int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    rclcpp::spin(std::make_shared<voicevox::VVSpeakerNode>());
    
    rclcpp::shutdown();
    return 0;
}