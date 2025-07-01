cmake_minimum_required(VERSION 3.8)
project(2d_face)

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
    --build-dir=${INOCHI2D_BUILD_DIR}/dub # build shenanigans
    --overide-config=targetPath=${INOCHI2D_BUILD_DIR}
  WORKING_DIRECTORY ${INOCHI2D_SOURCE_DIR}
  COMMENT "Building inochi2d-c with ${DUB_EXECUTABLE} (${LDC_EXECUTABLE}) into ${INOCHI2D_BUILD_DIR}"
)

add_custom_target(build-inochi2d-c ALL DEPENDS ${INOCHI2D_SO})

# now that it's built we finally add the library
add_library(inochi2d-c SHARED IMPORTED GLOBAL)
set_target_properties(inochi2d-c PROPERTIES
  IMPORTED_LOCATION ${INOCHI2D_SO}
  INTERFACE_INCLUDE_DIRECTORIES ${INOCHI2D_SOURCE_DIR}
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

# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

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
