//
// Created by cristi on 26.01.2026.
//

#include "../Include/controller.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "../Include/gpioChip.h"

Controller::Controller() :action(command::NONE), actionArgument(0), delta(0), timeSinceMPUUpdate(1000), timeSinceDistanceUpdate(1000) {
    UltrasonicDistance::Instance();
    MotorDriver::Instance()->setBias(-0.13);
    MPU6500::Instance();

    controlThread = std::thread(&Controller::controlThreadMain, this);
}

void Controller::turnoff() {
    actionMutex.lock();
    action = command::TURNOFF;
    actionMutex.unlock();
}

void Controller::turn(float deg) {
    actionMutex.lock();
    action = command::TURN;
    actionArgument.deg = deg;
    actionMutex.unlock();
}

void Controller::forward(int cm) {
    actionMutex.lock();
    action = command::FORWARD;
    actionArgument.cm = cm;
    actionMutex.unlock();
}

void Controller::backward(int cm) {
    actionMutex.lock();
    action = command::BACKWARD;
    actionArgument.cm = cm;
    actionMutex.unlock();
}

Controller::~Controller() {
    actionMutex.lock();
    if (action == command::NONE) {
        action = command::TURNOFF;
    }
    actionMutex.unlock();
    controlThread.join();
}

void Controller::controlThreadMain() {
    std::cout << "[THREAD STARTED]" << std::endl;
    while (true) {
        delta = deltaTime();
        // std::cout << delta << ' ' << timeSinceDistanceUpdate << std::endl;

        timeSinceMPUUpdate += delta;
        if (timeSinceMPUUpdate > MPU_CHECK_INTERVAL) {
            MPU6500::Instance()->update();
            timeSinceMPUUpdate = std::chrono::milliseconds(0);
        }

        timeSinceDistanceUpdate += delta;
        if (timeSinceDistanceUpdate > DISTANCE_CHECK_INTERVAL) {
            std::cout << "[DISTANCE]: " << UltrasonicDistance::Instance()->getDistance() << std::endl;
            UltrasonicDistance::Instance()->ping();
            timeSinceDistanceUpdate = std::chrono::milliseconds(0);
        }
        UltrasonicDistance::Instance()->update();

        actionMutex.lock();

        static float angle = 0;
        static float distanceTraveled = 0;
        switch (action) {
            case command::NONE:
                break;
            case command::TURNOFF:
                MotorDriver::ResetInstance();
                MPU6500::ResetInstance();
                UltrasonicDistance::ResetInstance();
                GpioChip::ResetInstance();
                action = command::NONE;
                actionMutex.unlock();
                std::cout << "[THREAD EXITED]" << std::endl;
                return;
            case command::FORWARD:
                if (!MotorDriver::Instance()->is_on()) {
                    distanceTraveled = 0;
                    MotorDriver::Instance()->setSpeedL(40);
                    MotorDriver::Instance()->setSpeedR(40);
                    MotorDriver::Instance()->setDirectionLF(true);
                    MotorDriver::Instance()->setDirectionRF(true);
                    MotorDriver::Instance()->setDirectionLB(true);
                    MotorDriver::Instance()->setDirectionRB(true);
                    MotorDriver::Instance()->startMotor();
                }

                distanceTraveled += (delta.count() / 1000000.0f) * -MPU6500::Instance()->speed[0] * 100;
                std::cout << "[TRAVELED]: " << distanceTraveled << std::endl;
                if (distanceTraveled > actionArgument.cm - 5) {
                    MotorDriver::Instance()->stopMotor();
                    action = command::NONE;
                }

                if (UltrasonicDistance::Instance()->getDistance() < 15) {
                    MotorDriver::Instance()->stopMotor();
                    action = command::NONE;
                }

                break;
            case command::TURN:
                if (!MotorDriver::Instance()->is_on()) {
                    angle = MPU6500::Instance()->angle[2]/ 0.01745 ;
                    if (actionArgument.deg > 0) {
                        MotorDriver::Instance()->setSpeedL(0);
                        MotorDriver::Instance()->setSpeedR(50);
                    } else {
                        MotorDriver::Instance()->setSpeedL(50);
                        MotorDriver::Instance()->setSpeedR(0);
                    }
                    MotorDriver::Instance()->setDirectionLF(true);
                    MotorDriver::Instance()->setDirectionRF(true);
                    MotorDriver::Instance()->setDirectionLB(true);
                    MotorDriver::Instance()->setDirectionRB(true);
                    MotorDriver::Instance()->startMotor();
                }
                // std::cout << angle << ' ' << MPU6500::Instance()->angle[2] / 0.01745  << ' ' << actionArgument.deg << std::endl;

                if (std::abs(angle - MPU6500::Instance()->angle[2] / 0.01745 ) > std::abs(actionArgument.deg) - 5) {
                    MotorDriver::Instance()->stopMotor();
                    action = command::NONE;
                }
                break;

            case command::BACKWARD:
                if (!MotorDriver::Instance()->is_on()) {
                    distanceTraveled = 0;
                    MotorDriver::Instance()->setSpeedL(50);
                    MotorDriver::Instance()->setSpeedR(50);
                    MotorDriver::Instance()->setDirectionLF(false);
                    MotorDriver::Instance()->setDirectionRF(false);
                    MotorDriver::Instance()->setDirectionLB(false);
                    MotorDriver::Instance()->setDirectionRB(false);
                    MotorDriver::Instance()->startMotor();
                }

                distanceTraveled += (delta.count() / 1000000.0f) * MPU6500::Instance()->speed[0] * 100;
                std::cout << "[TRAVELED]: " << distanceTraveled << std::endl;
                if (distanceTraveled > actionArgument.cm - 5) {
                    MotorDriver::Instance()->stopMotor();
                    action = command::NONE;
                }

                break;

        }
        actionMutex.unlock();
    }
    std::cout << "[THREAD EXITED]" << std::endl;
}

std::chrono::microseconds Controller::deltaTime() {
    static auto lastTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto delta = currentTime - lastTime;
    lastTime = currentTime;
    return std::chrono::duration_cast<std::chrono::microseconds>(delta);
}
