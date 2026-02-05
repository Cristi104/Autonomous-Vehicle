//
// Created by cristi on 22.01.2026.
//

#include "../Include/motorDriver.h"

#include <array>
#include <cstring>
#include <iostream>

#include "../Include/gpioChip.h"

#define MIN_DUTY_CYCLE 0
#define MAX_DUTY_CYCLE 100000

MotorDriver::MotorDriver(gpiod::chip &chip, unsigned int digitalLF, unsigned int digitalLB, unsigned int digitalRF, unsigned int digitalRB)
    :isOn(false), chip(chip), lf(digitalLF), lb(digitalLB), rf(digitalRF), rb(digitalRB), PWML(0, 0), PWMR(0, 1),
    bias(0.0), directionLF(false), directionLB(false), directionRF(false), directionRB(false), dutyL(0), dutyR(0), speedL(0), speedR(0){
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

MotorDriver * MotorDriver::instance = nullptr;
MotorDriver * MotorDriver::Instance() {
    if (instance == nullptr) {
        instance = new MotorDriver(GpioChip::Instance(), 20, 16, 26, 19);
    }
    return instance;
}

void MotorDriver::ResetInstance() {
    delete instance;
    instance = nullptr;
}

void MotorDriver::setDirectionLF(bool direction_lf) {
    directionLF = direction_lf;
    update();
}

void MotorDriver::setDirectionLB(bool direction_lb) {
    directionLB = direction_lb;
    update();
}

void MotorDriver::setDirectionRF(bool direction_rf) {
    directionRF = direction_rf;
    update();
}

void MotorDriver::setDirectionRB(bool direction_rb) {
    directionRB = direction_rb;
    update();
}

void MotorDriver::setSpeedL(int speed_l) {
    speedL = speed_l;
    dutyL = speed_l;
    dutyL *= 1000;
    dutyL *= 1.0f - bias;
    dutyL = std::max(std::min(dutyL, MAX_DUTY_CYCLE), MIN_DUTY_CYCLE);
    update();
}

void MotorDriver::setSpeedR(int speed_r) {
    speedR = speed_r;
    dutyR = speed_r;
    dutyR *= 1000;
    dutyR *= 1.0f + bias;
    dutyR = std::max(std::min(dutyR, MAX_DUTY_CYCLE), MIN_DUTY_CYCLE);
    update();
}

void MotorDriver::setBias(float bias) {
    this->bias = bias;
}

void MotorDriver::startMotor() {
    request->set_value(lf, boolToGpiod(directionLF));
    request->set_value(lb, boolToGpiod(directionLB));
    request->set_value(rf, boolToGpiod(directionRF));
    request->set_value(rb, boolToGpiod(directionRB));
    isOn = true;
    update();
    PWML.enable(true);
    PWMR.enable(true);
}

void MotorDriver::stopMotor() {
    PWML.enable(false);
    PWMR.enable(false);
    isOn = false;

    request->set_value(lf, boolToGpiod(false));
    request->set_value(lb, boolToGpiod(false));
    request->set_value(rf, boolToGpiod(false));
    request->set_value(rb, boolToGpiod(false));
}

void MotorDriver::update() {
    if (!(directionLF || directionLB)) {
        PWML.setDuty(dutyL);
    } else {
        PWML.setDuty(MAX_DUTY_CYCLE - dutyL);
    }
    if (!(directionRF || directionRB)) {
        PWMR.setDuty(dutyR);
    } else {
        PWMR.setDuty(MAX_DUTY_CYCLE - dutyR);
    }
    if (isOn) {
        request->set_value(lf, boolToGpiod(directionLF));
        request->set_value(lb, boolToGpiod(directionLB));
        request->set_value(rf, boolToGpiod(directionRF));
        request->set_value(rb, boolToGpiod(directionRB));
    }
}

int MotorDriver::getSpeedL() const {
    return speedL;
}

int MotorDriver::getSpeedR() const {
    return speedR;
}

bool MotorDriver::is_on() const {
    return isOn;
}

gpiod::line::value MotorDriver::boolToGpiod(bool value) {
    if (value) {
        return gpiod::line::value::ACTIVE;
    }
    return gpiod::line::value::INACTIVE;
}
