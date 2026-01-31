//
// Created by cristi on 22.01.2026.
//
#include <chrono>
#include <iostream>
#include <thread>

#include <gpiod.hpp>

#include "Include/controller.h"
#include "Include/mpu6500.h"
#include "Include/motorDriver.h"
#include "Include/ultrasonicDistance.h"

#include "pybind11/pybind11.h"

namespace py = pybind11;

PYBIND11_MODULE(controller, m, py::mod_gil_not_used()) {
    py::class_<Controller>(m, "Controller")
        .def(py::init())
        .def("turnoff", &Controller::turnoff)
        .def("turn", &Controller::turn)
        .def("forward", &Controller::forward)
        .def("backward", &Controller::backward);
}

int main() {
    Controller controller;
    // std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    controller.forward(20);
    // controller.turn(90);
    // controller.backward(20);
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    // MPU6500::Instance()->test();
    // MotorDriver::Instance()->setBias(-0.11);
    // MotorDriver::Instance()->setDirectionLF(true);
    // MotorDriver::Instance()->setDirectionRF(true);
    // MotorDriver::Instance()->setDirectionLB(true);
    // MotorDriver::Instance()->setDirectionRB(true);
    // MotorDriver::Instance()->setSpeedL(0);
    // MotorDriver::Instance()->setSpeedR(0);
    // MotorDriver::Instance()->startMotor();
    // MotorDriver::Instance()->setSpeedL(30);
    // MotorDriver::Instance()->setSpeedR(30);
    // MotorDriver::Instance()->stopMotor();
    // MPU6500 mpu;
    // PWM pwml(0, 0), pwmr(0, 1);
    // pwml.setPeriod(100000);
    // pwmr.setPeriod(100000);
    // pwml.setDuty(25000);
    // pwmr.setDuty(50000);
    // pwml.enable(true);
    // pwmr.enable(true)

    // UltrasonicDistance ultrasonic(chip, 8, 7);
    // for (int i=0; i<10; i++) {
    //     ultrasonic.ping();
    //     while (!ultrasonic.update());
    //     std::cout << ultrasonic.getDistance() << std::endl;
    //     std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // }

    // MotorDriver driver(chip, 20, 16, 26, 19);
    // driver.setBias(-0.11);
    // driver.setDirectionLF(true);
    // driver.setDirectionRF(true);
    // driver.setDirectionLB(true);
    // driver.setDirectionRB(true);
    // driver.setSpeedL(90);
    // driver.setSpeedR(90);
    // driver.startMotor();
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // driver.stopMotor();
    // // pwml.enable(false);
    // // pwmr.enable(false);
}