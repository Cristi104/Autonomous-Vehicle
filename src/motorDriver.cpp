//
// Created by cristi on 22.01.2026.
//

#include "../Include/motorDriver.h"

#include <array>
#include <cstring>
#include <iostream>


MotorDriver::MotorDriver(gpiod::chip &chip, unsigned int digitalLF, unsigned int digitalLB, unsigned int digitalRF, unsigned int digitalRB)
    :chip(chip), lf(digitalLF), lb(digitalLB), rf(digitalRF), rb(digitalRB), PWML(0, 0), PWMR(0, 1), directionLF(false), directionLB(false), directionRF(false), directionRB(false), speedL(0), speedR(0){
    gpiod::line::offsets offsets;
    offsets.push_back(lf);
    offsets.push_back(lb);
    offsets.push_back(rf);
    offsets.push_back(rb);

    gpiod::line_settings settings;
    settings.set_direction(gpiod::line::direction::OUTPUT);
    settings.set_output_value(gpiod::line::value::INACTIVE);

    gpiod::line_config config;
    config.add_line_settings(offsets, settings);

    gpiod::request_config req_cfg;
    req_cfg.set_consumer("MotorDriver");
    gpiod::request_builder builder = chip.prepare_request();
    builder.set_request_config(req_cfg);
    builder.set_line_config(config);

    request = builder.do_request();
}

void MotorDriver::setDirectionLF(bool direction_lf) {
    directionLF = direction_lf;
}

void MotorDriver::setDirectionLB(bool direction_lb) {
    directionLB = direction_lb;
}

void MotorDriver::setDirectionRF(bool direction_rf) {
    directionRF = direction_rf;
}

void MotorDriver::setDirectionRB(bool direction_rb) {
    directionRB = direction_rb;
}

void MotorDriver::setSpeedL(int speed_l) {
    speedL = speed_l;
}

void MotorDriver::setSpeedR(int speed_r) {
    speedR = speed_r;
}

void MotorDriver::startMotor() {
    request->set_value(lf, boolToGpiod(directionLF));
    request->set_value(lb, boolToGpiod(directionLB));
    request->set_value(rf, boolToGpiod(directionRF));
    request->set_value(rb, boolToGpiod(directionRB));
    update();
    PWML.enable(true);
    PWMR.enable(true);
}

void MotorDriver::stopMotor() {
    PWML.enable(false);
    PWMR.enable(false);

    request->set_value(lf, boolToGpiod(false));
    request->set_value(lb, boolToGpiod(false));
    request->set_value(rf, boolToGpiod(false));
    request->set_value(rb, boolToGpiod(false));
}

void MotorDriver::update() {
    if (directionLF || directionLB) {
        PWML.setDuty(speedL * 1000);
    } else {
        PWML.setDuty((100 - speedL) * 1000);
    }
    if (directionRF || directionRB) {
        PWMR.setDuty(speedR * 1000);
    } else {
        PWMR.setDuty((100 - speedR) * 1000);
    }
    request->set_value(lf, boolToGpiod(directionLF));
    request->set_value(lb, boolToGpiod(directionLB));
    request->set_value(rf, boolToGpiod(directionRF));
    request->set_value(rb, boolToGpiod(directionRB));
}

gpiod::line::value MotorDriver::boolToGpiod(bool value) {
    if (value) {
        return gpiod::line::value::ACTIVE;
    }
    return gpiod::line::value::INACTIVE;
}