cmake_minimum_required(VERSION 3.8)
project(face2d)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#-------------------------inochi2d stuff----------------------------

# find build tools
find_program(DUB_EXECUTABLE dub REQUIRED)
find_program(LDC_EXECUTABLE ldc2 REQUIRED)

set(INOCHI2D_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/inochi2d_build)
set(INOCHI2D_SO ${INOCHI2D_BUILD_DIR}/libinochi2d-c.so)
set(INOCHI2D_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/inochi2d-c)

# make the dir if it doesn't already exist
file(MAKE_DIRECTORY ${INOCHI2D_BUILD_DIR})

# now we build
add_custom_command(
  OUTPUT ${INOCHI2D_SO}
  COMMAND ${DUB_EXECUTABLE} build
    --compiler=${LDC_EXECUTABLE}
    --config=yesgl
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${INOCHI2D_SOURCE_DIR}/out/libinochi2d-c.so
    ${INOCHI2D_SO}
  WORKING_DIRECTORY ${INOCHI2D_SOURCE_DIR}
  COMMENT "Building inochi2d-c with ${DUB_EXECUTABLE} (${LDC_EXECUTABLE}) and copying into ${INOCHI2D_BUILD_DIR}"
)

add_custom_target(build-inochi2d-c ALL DEPENDS ${INOCHI2D_SO})

# now that it's built we finally add the library
add_library(inochi2d-c SHARED IMPORTED GLOBAL)
set_target_properties(inochi2d-c PROPERTIES
  IMPORTED_LOCATION ${INOCHI2D_SO}
  INTERFACE_INCLUDE_DIRECTORIES ${INOCHI2D_SOURCE_DIR}
)

install(FILES ${INOCHI2D_SO}
  DESTINATION lib
)

#--------------------------SFML stuff------------------------------

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

#target_link_libraries(main PRIVATE SFML::Graphics)

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(face2d 
  src/face2d.cpp
  src/face2d_node.cpp
  src/puppet_window.cpp
  src/camera.cpp
  src/parameter.cpp
)
# Ensure runtime linker can find the shared lib from install tree
#set_target_properties(face2d PROPERTIES
#  BUILD_RPATH "$ORIGIN/.."
#  INSTALL_RPATH "$ORIGIN/.."
#)
add_dependencies(face2d build-inochi2d-c)

ament_target_dependencies(face2d
  rclcpp
  ament_index_cpp
  # EXCLUDE_TARGET_LIBRARIES
)

target_link_libraries(face2d  
  SFML::Window
  OpenGL::GL
  inochi2d-c
)

target_include_directories(face2d
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)


install(
  DIRECTORY puppet
  DESTINATION share/${PROJECT_NAME}
)

install(TARGETS
  face2d
  DESTINATION lib/${PROJECT_NAME}
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
