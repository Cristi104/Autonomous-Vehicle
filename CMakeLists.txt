cmake_minimum_required(VERSION 3.31)
project(licenta)

set(CMAKE_CXX_STANDARD 20)

add_executable(licenta main.cpp
        src/mpu6500.cpp
        Include/mpu6500.h
        src/motorDriver.cpp
        Include/motorDriver.h
        src/PWM.cpp
        Include/PWM.h)

target_link_libraries(licenta PRIVATE gpiod)