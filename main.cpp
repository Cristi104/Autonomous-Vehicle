//
// Created by cristi on 22.01.2026.
//
#include <chrono>
#include <iostream>
#include <thread>

#include "Include/mpu6500.h"
#include "Include/motorDriver.h"
#include "Include/ultrasonicDistance.h"
#include <gpiod.hpp>


int main() {
    // MPU6500 mpu;
    // mpu.test();
    // PWM pwml(0, 0), pwmr(0, 1);
    // pwml.setPeriod(100000);
    // pwmr.setPeriod(100000);
    // pwml.setDuty(25000);
    // pwmr.setDuty(50000);
    // pwml.enable(true);
    // pwmr.enable(true)
    gpiod::chip chip(std::filesystem::path("/dev/gpiochip0"));

    UltrasonicDistance ultrasonic(chip, 8, 7);
    for (int i=0; i<10; i++) {
        ultrasonic.ping();
        while (!ultrasonic.update());
        std::cout << ultrasonic.getDistance() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // MotorDriver driver(chip, 20, 16, 26, 19);
    // driver.setDirectionLF(true);
    // driver.setDirectionRF(true);
    // driver.setSpeedL(50);
    // driver.setSpeedR(50);
    // driver.startMotor();
    // std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    // driver.stopMotor();
    // pwml.enable(false);
    // pwmr.enable(false);
}

// int main() {
//     PWM pwm0(0, 1); // pwmchip0, PWM0
//     // PWM pwm1(0, 1); // pwmchip0, PWM1
//
//     pwm0.setPeriod(20000000);
//     // pwm1.setPeriod(20000000);
//
//     pwm0.setDuty(10000000); // 5%
//     // pwm1.setDuty(1500000); // 7.5%
//
//     pwm0.enable(true);
//     // pwm1.enable(true);
//
//     sleep(5);
//
//     pwm0.enable(false);
//     // pwm1.enable(false);
// }
