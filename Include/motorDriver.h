//
// Created by cristi on 22.01.2026.
//

#ifndef LICENTA_MOTORDRIVER_H
#define LICENTA_MOTORDRIVER_H
#include <gpiod.hpp>
#include "PWM.h"


class MotorDriver {
public:
    static MotorDriver *Instance();
    static void ResetInstance();
    MotorDriver(const MotorDriver &other) = delete;
    MotorDriver(MotorDriver &&other) noexcept = delete;
    MotorDriver & operator=(const MotorDriver &other) = delete;
    MotorDriver & operator=(MotorDriver &&other) noexcept = delete;
    void setDirectionLF(bool direction_lf);
    void setDirectionLB(bool direction_lb);
    void setBias(float bias);
    void setDirectionRF(bool direction_rf);
    void setDirectionRB(bool direction_rb);
    void setSpeedL(int speed_l);
    void setSpeedR(int speed_r);
    ~MotorDriver() = default;
    void startMotor();
    void stopMotor();

private:
    MotorDriver(
        gpiod::chip &chip,
        unsigned int digitalLF, unsigned int digitalLB,
        unsigned int digitalRF, unsigned int digitalRB
        );

    void update();

    gpiod::chip &chip;
    std::optional<gpiod::line_request> request;
    unsigned int lf, lb, rf, rb;
    float bias;
    bool directionLF, directionLB, directionRF, directionRB;
    int speedL, speedR;
    bool isOn;

public:
    [[nodiscard]] bool is_on() const;

private:
    PWM PWML, PWMR;

    static gpiod::line::value boolToGpiod(bool value);

    static MotorDriver *instance;
};


#endif //LICENTA_MOTORDRIVER_H