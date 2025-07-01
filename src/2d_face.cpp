#include <memory>

#include "rclcpp/rclcpp.hpp"

#include <SFML/Window.hpp>
#include <inochi2d.h>
class Face2DNode : public rclcpp::Node
{
public:
    Face2DNode()
    : Node("face2d")
    {
        RCLCPP_INFO(get_logger(), "Hello world!");
    }
}; 

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<Face2DNode>());
    rclcpp::shutdown();
    return 0;
}