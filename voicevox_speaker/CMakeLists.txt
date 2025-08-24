cmake_minimum_required(VERSION 3.8)
project(voicevox_speaker)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

#------------------------Voicevox stuff----------------------------
# paths
set(VOICEVOX_SRC ${CMAKE_CURRENT_SOURCE_DIR}/external/voicevox_core)
set(VOICEVOX_HEADERS_SRC ${VOICEVOX_SRC}/c_api/include)
set(VOICEVOX_HEADERS_TARGET ${CMAKE_CURRENT_SOURCE_DIR}/include/voicevox_core)
set(VOICEVOX_SO_SRC ${VOICEVOX_SRC}/c_api/lib/libvoicevox_core.so)

# copy to build
set(VOICEVOX_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/voicevox_build) 
file(MAKE_DIRECTORY ${VOICEVOX_BUILD_DIR})
set(VOICEVOX_SO_BUILD ${VOICEVOX_BUILD_DIR}/libvoicevox_core.so)

# copy headers
file(MAKE_DIRECTORY ${VOICEVOX_HEADERS_TARGET})
add_custom_command(
  OUTPUT ${VOICEVOX_HEADERS_TARGET}/voicevox_core.h
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${VOICEVOX_HEADERS_SRC}/voicevox_core.h
    ${VOICEVOX_HEADERS_TARGET}/voicevox_core.h
  DEPENDS ${VOICEVOX_SO_SRC}
  COMMENT "Copying voicevox_core.h into src"
)

# copy lib into build dir
add_custom_command(
  OUTPUT ${VOICEVOX_SO_BUILD}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${VOICEVOX_SO_SRC}
    ${VOICEVOX_SO_BUILD}
  DEPENDS ${VOICEVOX_SO_SRC}
  COMMENT "Copying voicevox_core.so into build tree"
)

add_custom_target(voicevox_core_copy ALL 
  DEPENDS 
    ${VOICEVOX_SO_BUILD}
    ${VOICEVOX_HEADERS_TARGET}/voicevox_core.h
)

# now we add the library
add_library(voicevox_core SHARED IMPORTED GLOBAL)
set_target_properties(voicevox_core PROPERTIES
  IMPORTED_LOCATION ${VOICEVOX_SO_BUILD}
  IMPORTED_NO_SONAME TRUE
  INTERFACE_INCLUDE_DIRECTORIES ${VOICEVOX_HEADERS_SRC}
)

add_dependencies(voicevox_core voicevox_core_copy)

# install .so and headers 
install(FILES ${VOICEVOX_SO_BUILD} DESTINATION lib)
install(DIRECTORY ${VOICEVOX_HEADERS_SRC}/ DESTINATION include/voicevox_core)

# install the models + dict
install(
  DIRECTORY ${VOICEVOX_SRC}/models ${VOICEVOX_SRC}/dict
  DESTINATION share/${PROJECT_NAME}
)

# install onnx runtime libs
install(DIRECTORY ${VOICEVOX_SRC}/onnxruntime/lib/
  DESTINATION share/${PROJECT_NAME}/onnx_lib
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

# -------------------------------ROS2-----------------------------

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(speaker_actions REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

add_executable(speaker_node
  src/voicevox_utils.cpp
  src/speaker_node.cpp
  src/synthesizer.cpp
  src/voicevox.cpp
  src/voicevox_speaker.cpp
)
add_dependencies(speaker_node voicevox_core)

ament_target_dependencies(speaker_node
  ament_index_cpp
  rclcpp
  speaker_actions
)

target_link_libraries(speaker_node
  voicevox_core
  SFML::Audio
)

target_include_directories(speaker_node
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS
  speaker_node
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
