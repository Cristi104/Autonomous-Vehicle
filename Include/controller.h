//
// Created by cristi on 26.01.2026.
//

#ifndef LICENTA_CONTROLLER_H
#define LICENTA_CONTROLLER_H
#include <thread>

#include "motorDriver.h"
#include "mpu6500.h"
#include "ultrasonicDistance.h"


#define DISTANCE_CHECK_INTERVAL std::chrono::milliseconds(150)
#define MPU_CHECK_INTERVAL std::chrono::milliseconds(25)

class Controller {
public:
    Controller();
    void startThread();
    void stopThread();
    void forwardWithSpeed(int speed);
    void forwardCm(int cm);
    void turnWithSpeed(int speed);
    void turnDeg(float deg);
    void pid(float steering);
    void setSpeed(int speed);
    void setPID(float kp, float ki, float kd, float setpoint);
    ~Controller();

private:
    void interrupt();
    std::atomic<bool> running;

    enum class command {
        NONE,
        FORWARD_CM,
        FORWARD_SPEED,
        TURN_DEG,
        TURN_SPEED,
        PID,
    };
    int speed;
    float kp, ki, kd, setpoint;

    std::atomic<uint64_t> interruptToken;

    std::mutex actionMutex;
    command action;
    int forwardSpeed;
    int forwardCentimeters;
    float turnDegrees;
    int turnSpeed;
    float pidSteering;

    void controlThreadMain();
    std::thread controlThread;

    std::chrono::microseconds deltaTime();
    std::chrono::microseconds delta;
    std::chrono::microseconds timeSinceMPUUpdate;
    std::chrono::microseconds timeSinceDistanceUpdate;
};


#endif //LICENTA_CONTROLLER_H