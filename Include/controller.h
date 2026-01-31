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
    void turnoff();
    void turn(float deg);
    void forward(int cm);
    void backward(int cm);
    ~Controller();

private:
    enum class command {
        NONE,
        FORWARD,
        BACKWARD,
        TURN,
        DONE,
        TURNOFF,
    };

    command action;
    union {
        int cm;
        float deg;
    } actionArgument;

    std::mutex actionMutex;
    void controlThreadMain();
    std::thread controlThread;

    std::chrono::microseconds deltaTime();
    std::chrono::microseconds delta;
    std::chrono::microseconds timeSinceMPUUpdate;
    std::chrono::microseconds timeSinceDistanceUpdate;

};


#endif //LICENTA_CONTROLLER_H