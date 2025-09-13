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
find_package(rclcpp_lifecycle REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(speaker_actions REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(face_msgs REQUIRED)
find_package(pluginlib REQUIRED)

# Tweeny
find_path(TWEENY_INCLUDE_DIR tweeny)

# Interfaces
add_library(expression_interfaces SHARED src/dummy.cpp)
target_include_directories(expression_interfaces PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${TWEENY_INCLUDE_DIR}
)

ament_export_targets(export_expression_interfaces HAS_LIBRARY_TARGET)
install(TARGETS expression_interfaces
  EXPORT expression_interfaces
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(TARGETS
expression_interfaces
EXPORT export_expression_interfaces
)

# Server
add_library(expression_server SHARED
  src/expression.cpp
  src/expression_node.cpp
  src/grouped_channels.cpp
  src/eye_channels.cpp
  src/mouth_channels.cpp
  src/neck_channels.cpp
)

target_include_directories(expression_server PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${TWEENY_INCLUDE_DIR}
)
target_compile_definitions(expression_server
  PRIVATE "CUSTOM_ACTION_CPP_BUILDING_DLL"
)
ament_target_dependencies(expression_server
  ament_index_cpp
  rclcpp
  rclcpp_action
  rclcpp_lifecycle
  rclcpp_components
  lifecycle_msgs
  speaker_actions
  face_msgs
  pluginlib
)

rclcpp_components_register_node(expression_server
  PLUGIN "expression::ExpressionNode"
  EXECUTABLE expression_node
)
install(TARGETS
  expression_server
  EXPORT export_expression_server
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)
ament_export_include_directories(
  include
  ${TWEENY_INCLUDE_DIR}
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
