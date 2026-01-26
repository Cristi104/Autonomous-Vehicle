//
// Created by cristi on 22.01.2026.
//

#ifndef LICENTA_MOTORDRIVER_H
#define LICENTA_MOTORDRIVER_H
#include <gpiod.hpp>
#include "PWM.h"


class MotorDriver {
public:
    MotorDriver(
        gpiod::chip &chip,
        unsigned int digitalLF, unsigned int digitalLB,
        unsigned int digitalRF, unsigned int digitalRB
        );
    void setDirectionLF(bool direction_lf);
    void setDirectionLB(bool direction_lb);
    void setDirectionRF(bool direction_rf);
    void setDirectionRB(bool direction_rb);
    void setSpeedL(int speed_l);
    void setSpeedR(int speed_r);
    ~MotorDriver() = default;
    void startMotor();
    void stopMotor();

private:
    gpiod::chip &chip;
    std::optional<gpiod::line_request> request;
    unsigned int lf, lb, rf, rb;
    bool directionLF, directionLB, directionRF, directionRB;
    int speedL, speedR;
    void update();
    static gpiod::line::value boolToGpiod(bool value);
    PWM PWML, PWMR;
};


#endif //LICENTA_MOTORDRIVER_H