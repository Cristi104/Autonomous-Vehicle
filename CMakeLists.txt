cmake_minimum_required(VERSION 3.31)
project(licenta)

set(CMAKE_CXX_STANDARD 20)


file(GLOB sources
    "Include/*.h"
    "src/*.cpp"
)

    
add_executable(licenta ${sources} main.cpp)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-app-1.0)

target_link_libraries(licenta PRIVATE Python3::Python)
#set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(controller ${sources} main.cpp)

install(TARGETS controller DESTINATION .)
target_link_libraries(controller PRIVATE gpiodcxx)

target_include_directories(licenta PRIVATE extern/pybind11/include)
target_include_directories(licenta PRIVATE extern/uWebSockets/src)
target_include_directories(licenta PRIVATE /usr/include/opencv4)
target_include_directories(licenta PRIVATE /usr/include/opencv4/opencv2)
target_include_directories(licenta PRIVATE ${OpenCV_INCLUDE_DIRS})
target_include_directories(licenta PRIVATE ${GSTREAMER_INCLUDE_DIRS})
target_link_directories(licenta PRIVATE ${GSTREAMER_LIBRARY_DIRS})
target_link_libraries(licenta PRIVATE gpiodcxx ssl crypto z pthread ${OpenCV_LIBS} ${GSTREAMER_LIBRARIES})


