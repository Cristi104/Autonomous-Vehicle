cmake_minimum_required(VERSION 3.31)
project(licenta)

set(CMAKE_CXX_STANDARD 20)



add_executable(licenta main.cpp
        src/mpu6500.cpp
        Include/mpu6500.h
        src/motorDriver.cpp
        Include/motorDriver.h
        src/PWM.cpp
        Include/PWM.h
        src/ultrasonicDistance.cpp
        Include/ultrasonicDistance.h
        src/controller.cpp
        Include/controller.h
        src/gpioChip.cpp
        Include/gpioChip.h)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)

target_link_libraries(licenta PRIVATE Python3::Python)
#set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(controller main.cpp
        src/mpu6500.cpp
        Include/mpu6500.h
        src/motorDriver.cpp
        Include/motorDriver.h
        src/PWM.cpp
        Include/PWM.h
        src/ultrasonicDistance.cpp
        Include/ultrasonicDistance.h
        src/controller.cpp
        Include/controller.h
        src/gpioChip.cpp
        Include/gpioChip.h)
install(TARGETS controller DESTINATION .)

target_link_libraries(controller PRIVATE gpiodcxx)
target_include_directories(licenta PRIVATE extern/pybind11/include)
target_link_libraries(licenta PRIVATE gpiodcxx)