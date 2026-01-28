//
// Created by cristi on 26.01.2026.
//

#include "../Include/controller.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "../Include/gpioChip.h"

Controller::Controller() :controlThread(&Controller::controlThreadMain, this), action(command::NONE), actionArgument(0), delta(0), timeSinceMPUUpdate(1000), timeSinceDistanceUpdate(1000) {
    MPU6500::Instance()->update();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    MPU6500::Instance()->update();
}

void Controller::turnoff() {
    actionMutex.lock();
    action = command::TURNOFF;
    actionMutex.unlock();
}

void Controller::turn(float deg) {
    actionMutex.lock();
    action = command::TURN;
    actionMutex.unlock();
    actionArgument.deg = deg;
}

void Controller::forward(int cm) {
    actionMutex.lock();
    action = command::FORWARD;
    actionMutex.unlock();
    actionArgument.cm = cm;
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
                    MotorDriver::Instance()->setSpeedL(40);
                    MotorDriver::Instance()->setSpeedR(40);
                    MotorDriver::Instance()->setDirectionLF(true);
                    MotorDriver::Instance()->setDirectionRF(true);
                    MotorDriver::Instance()->setDirectionLB(true);
                    MotorDriver::Instance()->setDirectionRB(true);
                    MotorDriver::Instance()->startMotor();
                }

                if (UltrasonicDistance::Instance()->getDistance() < 10) {
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
