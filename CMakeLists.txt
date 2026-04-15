cmake_minimum_required(VERSION 3.31)
project(licenta)

set(CMAKE_CXX_STANDARD 20)


file(GLOB_RECURSE sources
    "Include/*.h"
    "src/*.cpp"
)

add_executable(licenta ${sources} main.cpp)

# uWebSockets
include(FetchContent)

FetchContent_Declare(
  uSockets_content
  GIT_REPOSITORY https://github.com/uNetworking/uSockets
  GIT_TAG v0.8.5
  GIT_SHALLOW ON
  GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(uSockets_content)
file(GLOB_RECURSE SOURCES ${usockets_content_SOURCE_DIR}/src/*.c)
add_library(uSockets ${SOURCES})
target_include_directories(uSockets PUBLIC ${usockets_content_SOURCE_DIR}/src)
target_compile_definitions(uSockets PRIVATE LIBUS_NO_SSL)

FetchContent_Declare(
  uWebSockets_content
  GIT_REPOSITORY https://github.com/uNetworking/uWebSockets
  GIT_TAG v20.37.0
  GIT_SHALLOW ON
  GIT_SUBMODULES ""
)
FetchContent_MakeAvailable(uWebSockets_content)
find_package(ZLIB REQUIRED)
add_library(uWebSockets INTERFACE)
target_include_directories(uWebSockets INTERFACE ${uwebsockets_content_SOURCE_DIR}/src/)
target_link_libraries(uWebSockets INTERFACE uSockets ${ZLIB_LIBRARIES})
target_compile_options(uWebSockets INTERFACE -Wno-deprecated-declarations)
target_link_libraries(licenta PRIVATE uWebSockets)

# pybind
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG REQUIRED)
target_link_libraries(licenta PRIVATE Python3::Python)
pybind11_add_module(controller ${sources} main.cpp)
target_include_directories(licenta PRIVATE extern/pybind11/include)
target_link_libraries(controller PRIVATE gpiodcxx)
    
# opencv
find_package(OpenCV REQUIRED)
target_include_directories(licenta PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(licenta PRIVATE ${OpenCV_LIBS})

# gstreamer(for camera)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0)
target_include_directories(licenta PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_link_directories(licenta PRIVATE ${GSTREAMER_LIBRARY_DIRS})
target_link_libraries(licenta PRIVATE ${GSTREAMER_LIBRARIES})

# gpiodcxx
target_link_libraries(licenta PRIVATE gpiodcxx)

