cmake_minimum_required(VERSION 3.10)
project(face2d)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_BUILD_TYPE Debug)
# set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#----------------------------SFML stuff----------------------------

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM)
FetchContent_MakeAvailable(SFML)

# Use GLVND libraries if available
if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

# also tell CMake which to prefer
set(OpenGL_GL_PREFERENCE GLVND)

#-------------------------inochi2d stuff----------------------------

# Detect platform and arch
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64")
    set(LDC_ARCH "linux-x86_64")
    set(LDC_EXT "tar.xz")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(LDC_ARCH "linux-aarch64")
    set(LDC_EXT "tar.xz")
  else()
    message(FATAL_ERROR "Unsupported Linux architecture: ${CMAKE_SYSTEM_PROCESSOR}")
  endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(LDC_ARCH "osx-universal")
  set(LDC_EXT "tar.xz")

elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # Windows archives are 7z format and multilib
  set(LDC_ARCH "windows-multilib")
  set(LDC_EXT "7z")

else()
  message(FATAL_ERROR "Unsupported OS: ${CMAKE_SYSTEM_NAME}")
endif()

# Fetch LDC (since apt is outdated as of writing, Sep 2025)
set(LDC_VERSION "1.41.0")
set(LDC_URL "https://github.com/ldc-developers/ldc/releases/download/v${LDC_VERSION}/ldc2-${LDC_VERSION}-${LDC_ARCH}.${LDC_EXT}")
set(LDC_ARCHIVE "${CMAKE_BINARY_DIR}/ldc-${LDC_VERSION}.${LDC_EXT}")
set(LDC_EXTRACTED "${CMAKE_BINARY_DIR}/ldc-${LDC_VERSION}")

# Download + extract if not already present
if(NOT EXISTS "${LDC_EXTRACTED}")
  message(STATUS "Downloading LDC ${LDC_VERSION} for ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}...")
  file(DOWNLOAD "${LDC_URL}" "${LDC_ARCHIVE}" SHOW_PROGRESS)

  message(STATUS "Extracting LDC ${LDC_VERSION}...")
  if(LDC_EXT STREQUAL "tar.xz")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xzf "${LDC_ARCHIVE}"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  elseif(LDC_EXT STREQUAL "7z")
    # 7z not handled natively by cmake -E, require `7z` in PATH
    execute_process(
      COMMAND 7z x "${LDC_ARCHIVE}"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
  endif()

  # Folder inside archive looks like ldc2-1.41.0-<arch>
  file(GLOB LDC_DIRS "${CMAKE_BINARY_DIR}/ldc2-${LDC_VERSION}-${LDC_ARCH}")
  file(RENAME "${LDC_DIRS}" "${LDC_EXTRACTED}")
endif()

# find build tools
set(LDC_EXECUTABLE "${LDC_EXTRACTED}/bin/ldc2")
set(DUB_EXECUTABLE "${LDC_EXTRACTED}/bin/dub")

set(INOCHI2D_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/inochi2d_build)
set(INOCHI2D_SO ${INOCHI2D_BUILD_DIR}/libinochi2d.so)
set(INOCHI2D_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/inochi2d)

# make the dir if it doesn't already exist
file(MAKE_DIRECTORY ${INOCHI2D_BUILD_DIR})

# now we build
add_custom_command(
  OUTPUT ${INOCHI2D_SO}
  COMMAND ${DUB_EXECUTABLE} build
    --compiler=${LDC_EXECUTABLE}
    --build=release-debug
    --config=dynamic
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${INOCHI2D_SOURCE_DIR}/out/libinochi2d.so
    ${INOCHI2D_SO}
  WORKING_DIRECTORY ${INOCHI2D_SOURCE_DIR}
  COMMENT "Building inochi2d with ${DUB_EXECUTABLE} (${LDC_EXECUTABLE}) and copying into ${INOCHI2D_BUILD_DIR}"
)

add_custom_target(build-inochi2d ALL DEPENDS ${INOCHI2D_SO})

# now that it's built we finally add the library
add_library(inochi2d SHARED IMPORTED GLOBAL)
set_target_properties(inochi2d PROPERTIES
  IMPORTED_LOCATION ${INOCHI2D_SO}
  INTERFACE_INCLUDE_DIRECTORIES ${INOCHI2D_SOURCE_DIR}/res
)

install(FILES ${INOCHI2D_SO}
  DESTINATION lib
)

#-------------------------Dear imGui stuff-------------------------

# imgui
add_library(imgui STATIC
  external/imgui/imgui.cpp
  external/imgui/imgui_widgets.cpp
  external/imgui/imgui_tables.cpp
  external/imgui/imgui_demo.cpp
)

target_include_directories(imgui
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui
)

# imgui file dialog
add_library(imgui_file_dialog STATIC
  external/ImGuiFileDialog/ImGuiFileDialog.cpp
)

target_include_directories(imgui_file_dialog
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/external/ImGuiFileDialog
)

target_link_libraries(imgui_file_dialog 
  PUBLIC 
    imgui
)

# imgui-sfml
# add_library(imgui_sfml SHARED
#   external/imgui-sfml/imgui-SFML.cpp
# )

# target_include_directories(imgui_sfml
#   PUBLIC
#     ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui   # your ImGui
#     ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui-sfml
# )

# target_link_libraries(imgui_sfml
#   PUBLIC
#     SFML::Graphics
#     OpenGL::GL
#     imgui
# )

if(WIN32 AND MINGW)
  target_link_libraries(imgui_sfml PUBLIC imm32)
endif()
#-------------------------------ROS2-------------------------------

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)
find_package(OpenGL REQUIRED)
find_package(face_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)

add_executable(face2d 
  src/face2d.cpp
  src/face2d_node.cpp
  src/puppet_window.cpp
  src/camera.cpp
  src/parameter.cpp
)

add_dependencies(face2d build-inochi2d)

ament_target_dependencies(face2d
  rclcpp
  rclcpp_lifecycle
  ament_index_cpp
  lifecycle_msgs
  face_msgs
  yaml-cpp
)

target_link_libraries(face2d  
  SFML::Window
  SFML::Graphics
  OpenGL::GL
  inochi2d
  yaml-cpp
  imgui
  # imgui_sfml
  imgui_file_dialog
)

target_include_directories(face2d
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(
  DIRECTORY 
    puppet
    fonts
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
