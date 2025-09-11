cmake_minimum_required(VERSION 3.8)
project(expression)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Enabled C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(speaker_actions REQUIRED)
find_package(face_msgs REQUIRED)
find_package(behaviortree_cpp REQUIRED)

add_library(expression_server SHARED
  src/expression.cpp
  src/expression_node.cpp
  src/eye_publishers.cpp
  src/mouth_publishers.cpp
  src/neck_publishers.cpp
  src/breath_animation.cpp
)

target_include_directories(expression_server PRIVATE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/tweeny/include>
)
target_compile_definitions(expression_server
  PRIVATE "CUSTOM_ACTION_CPP_BUILDING_DLL"
)
ament_target_dependencies(expression_server
  ament_index_cpp
  rclcpp
  rclcpp_action
  rclcpp_components
  speaker_actions
  face_msgs
  behaviortree_cpp
)

rclcpp_components_register_node(expression_server
  PLUGIN "expression::ExpressionNode"
  EXECUTABLE expression_node
)
install(TARGETS
  expression_server
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)
install(
  DIRECTORY behavior_trees
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
