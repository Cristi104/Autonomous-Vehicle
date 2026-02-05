//
// Created by cristi on 26.01.2026.
//

#include "../Include/controller.h"
#include <chrono>
#include <iostream>
#include <thread>
#include "../Include/gpioChip.h"

Controller::Controller()
    :delta(0), timeSinceMPUUpdate(1000), timeSinceDistanceUpdate(1000), running(false), speed(50),
    interruptToken(0), action(command::NONE), forwardCentimeters(0), forwardSpeed(0),
    turnDegrees(0), turnSpeed(0), pidSteering(0), ki(0), kd(0), kp(1), setpoint(0)
{
    UltrasonicDistance::Instance();
    MotorDriver::Instance()->setBias(0);
    MPU6500::Instance();
}

void Controller::startThread() {
    if (!running) {
        running = true;
        controlThread = std::thread(&Controller::controlThreadMain, this);
    }
}

void Controller::stopThread() {
    if (running) {
        running = false;
        controlThread.join();
    }
}

void Controller::forwardWithSpeed(int speed) {
    actionMutex.lock();
    action = command::FORWARD_SPEED;
    forwardSpeed = speed;
    actionMutex.unlock();
}

void Controller::forwardCm(int cm) {
    actionMutex.lock();
    action = command::FORWARD_CM;
    forwardCentimeters = cm;
    actionMutex.unlock();
}

void Controller::turnWithSpeed(int speed) {
    actionMutex.lock();
    action = command::TURN_SPEED;
    turnSpeed = speed;
    actionMutex.unlock();
}

void Controller::turnDeg(float deg) {
    actionMutex.lock();
    action = command::TURN_DEG;
    turnDegrees = deg;
    actionMutex.unlock();
}

void Controller::pid(float steering) {
    actionMutex.lock();
    action = command::PID;
    pidSteering = steering;
    actionMutex.unlock();
}

Controller::~Controller() {
    if (running.load(std::memory_order_relaxed)) {
        stopThread();
    }
    UltrasonicDistance::ResetInstance();
    MotorDriver::Instance()->stopMotor();
    MotorDriver::ResetInstance();
    MPU6500::ResetInstance();
}

void Controller::interrupt() {
    interruptToken.fetch_add(1, std::memory_order_release);
}

void Controller::setSpeed(int speed) {
    if (speed > 100)
        this->speed = 100;
    if (speed < 30)
        this->speed = 30;
    if (speed >= 30 && speed <= 100)
        this->speed = speed;
}

void Controller::setPID(float kp, float ki, float kd, float setpoint) {
    this->kp = kp;
    this->kd = kd;
    this->ki = ki;
    this->setpoint = setpoint;
}

void Controller::controlThreadMain() {
    uint64_t observedCancel = interruptToken.load(std::memory_order_acquire);
    std::cout << "[THREAD STARTED]" << std::endl;

    float angle = 0;
    float distanceTraveled = 0;
    int lastSpeed = 0;
    float lastSteering = 0.0f;
    float integral = 0;
    float prev_error = 0;
    float p, i, d, error, totalPID;
    auto lastPIDTime = std::chrono::steady_clock::now();
    auto currentPIDTime = std::chrono::steady_clock::now();
    auto deltaPID = currentPIDTime - lastPIDTime;

    while (running.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
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

        uint64_t currentCancel = interruptToken.load(std::memory_order_acquire);
        if (observedCancel !=currentCancel) {
            observedCancel = currentCancel;
            MotorDriver::Instance()->stopMotor();

            actionMutex.lock();
            action = command::NONE;
            angle = 0;
            distanceTraveled = 0;
            lastSpeed = 0;
            lastSteering = 0;
            integral = 0;
            prev_error = 0;
            actionMutex.unlock();

            continue;
        }

        actionMutex.lock();
        switch (action) {
            case command::NONE:
                break;
            case command::FORWARD_CM:
                if (!MotorDriver::Instance()->is_on()) {
                    distanceTraveled = 0;
                    MotorDriver::Instance()->setSpeedL(speed);
                    MotorDriver::Instance()->setSpeedR(speed);
                    MotorDriver::Instance()->setDirectionLF(true);
                    MotorDriver::Instance()->setDirectionRF(true);
                    MotorDriver::Instance()->setDirectionLB(true);
                    MotorDriver::Instance()->setDirectionRB(true);
                    MotorDriver::Instance()->startMotor();
                }

                distanceTraveled += (delta.count() / 1000000.0f) * -MPU6500::Instance()->speed[0] * 100;
                std::cout << "[TRAVELED]: " << distanceTraveled << std::endl;
                if (distanceTraveled > forwardCentimeters - 5) {
                    MotorDriver::Instance()->stopMotor();
                    action = command::NONE;
                }

                if (UltrasonicDistance::Instance()->getDistance() < 15 && UltrasonicDistance::Instance()->getDistance() > 1) {
                    MotorDriver::Instance()->stopMotor();
                    action = command::NONE;
                }

                break;
            case command::FORWARD_SPEED:
                if (lastSpeed != forwardSpeed) {
                    MotorDriver::Instance()->setSpeedL(forwardSpeed);
                    MotorDriver::Instance()->setSpeedR(forwardSpeed);
                    lastSpeed = forwardSpeed;
                }

                if (!MotorDriver::Instance()->is_on()) {
                    distanceTraveled = 0;
                    MotorDriver::Instance()->setDirectionLF(true);
                    MotorDriver::Instance()->setDirectionRF(true);
                    MotorDriver::Instance()->setDirectionLB(true);
                    MotorDriver::Instance()->setDirectionRB(true);
                    MotorDriver::Instance()->setSpeedL(forwardSpeed);
                    MotorDriver::Instance()->setSpeedR(forwardSpeed);
                    lastSpeed = forwardSpeed;
                    MotorDriver::Instance()->startMotor();
                }

                if (UltrasonicDistance::Instance()->getDistance() < 15 && UltrasonicDistance::Instance()->getDistance() > 1) {
                    MotorDriver::Instance()->stopMotor();
                    action = command::NONE;
                }

                break;
            case command::PID:
                if (!MotorDriver::Instance()->is_on()) {
                    MotorDriver::Instance()->setDirectionLF(true);
                    MotorDriver::Instance()->setDirectionRF(true);
                    MotorDriver::Instance()->setDirectionLB(true);
                    MotorDriver::Instance()->setDirectionRB(true);
                    MotorDriver::Instance()->startMotor();
                    MotorDriver::Instance()->setSpeedL(speed * (1.0f + pidSteering));
                    MotorDriver::Instance()->setSpeedR(speed * (1.0f - pidSteering));
                    lastSteering = pidSteering;
                }

                if (lastSteering != pidSteering) {
                    currentPIDTime = std::chrono::steady_clock::now();
                    deltaPID = currentPIDTime - lastPIDTime;
                    lastPIDTime = currentPIDTime;

                    error = setpoint - pidSteering;
                    p = kp * error;
                    i = ki * integral;
                    d = kd * (error - prev_error) / (std::chrono::duration_cast<std::chrono::microseconds>(deltaPID).count() / 1000000.0f);

                    totalPID = p +i + d;

                    MotorDriver::Instance()->setSpeedL(speed * (1.0f + totalPID));
                    MotorDriver::Instance()->setSpeedR(speed * (1.0f - totalPID));
                    lastSteering = pidSteering;
                }

                if (UltrasonicDistance::Instance()->getDistance() < 15 && UltrasonicDistance::Instance()->getDistance() > 1) {
                    MotorDriver::Instance()->stopMotor();
                    action = command::NONE;
                }

                break;
            case command::TURN_DEG:
                if (!MotorDriver::Instance()->is_on()) {
                    angle = MPU6500::Instance()->angle[2]/ 0.01745f ;
                    if (turnDegrees > 0) {
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

                if (std::abs(angle - MPU6500::Instance()->angle[2] / 0.01745 ) > std::abs(turnDegrees) - 5) {
                    MotorDriver::Instance()->stopMotor();
                    action = command::NONE;
                }

                break;
            case command::TURN_SPEED:
                if (lastSpeed != turnSpeed) {
                    if (turnSpeed > 0) {
                        MotorDriver::Instance()->setSpeedL(0);
                        MotorDriver::Instance()->setSpeedR(turnSpeed);
                    } else {
                        MotorDriver::Instance()->setSpeedL(-turnSpeed);
                        MotorDriver::Instance()->setSpeedR(0);
                    }
                    lastSpeed = turnSpeed;
                }
                if (!MotorDriver::Instance()->is_on()) {
                    if (turnSpeed > 0) {
                        MotorDriver::Instance()->setSpeedL(0);
                        MotorDriver::Instance()->setSpeedR(turnSpeed);
                    } else {
                        MotorDriver::Instance()->setSpeedL(-turnSpeed);
                        MotorDriver::Instance()->setSpeedR(0);
                    }
                    lastSpeed = turnSpeed;
                    MotorDriver::Instance()->setDirectionLF(true);
                    MotorDriver::Instance()->setDirectionRF(true);
                    MotorDriver::Instance()->setDirectionLB(true);
                    MotorDriver::Instance()->setDirectionRB(true);
                    MotorDriver::Instance()->startMotor();
                }

                break;
        }
        actionMutex.unlock();
    }
    MotorDriver::Instance()->stopMotor();
    std::cout << "[THREAD EXITED]" << std::endl;
}

std::chrono::microseconds Controller::deltaTime() {
    static auto lastTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto delta = currentTime - lastTime;
    lastTime = currentTime;
    return std::chrono::duration_cast<std::chrono::microseconds>(delta);
}
