#include "face2d/face2d_node.hpp"

#include "rclcpp/rclcpp.hpp"


int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);

    rclcpp::spin(std::make_shared<face2d::Face2DNode>());
    
    rclcpp::shutdown();
    return 0;
}