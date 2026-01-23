//
// Created by cristi on 22.01.2026.
//

#include "../Include/motorDriver.h"

#include <array>
#include <cstring>
#include <iostream>


MotorDriver::MotorDriver(unsigned int digitalLF, unsigned int digitalLB, unsigned int digitalRF, unsigned int digitalRB)
    :lf(digitalLF), lb(digitalLB), rf(digitalRF), rb(digitalRB), PWML(0, 0), PWMR(0, 1), directionLF(false), directionLB(false), directionRF(false), directionRB(false), speedL(0), speedR(0){
    chip = gpiod_chip_open("/dev/gpiochip0");
    if (!chip) {
        std::cerr << "Failed to open gpiochip0" << std::endl;
        return;
    }

    unsigned int offsets[] = { lf, lb, rf, rb };

    gpiod_line_settings* settings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

    gpiod_line_config* config = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(config, offsets, 4, settings);

    gpiod_request_config* req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "MotorDriver");

    request = gpiod_chip_request_lines(chip, req_cfg, config);

    if (!request) {
        std::cerr << "Failed to request GPIO lines" << std::endl;
    }

    gpiod_line_settings_free(settings);
    gpiod_line_config_free(config);
    gpiod_request_config_free(req_cfg);
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

MotorDriver::~MotorDriver() {
    if (request)
        gpiod_line_request_release(request);
    if (chip)
        gpiod_chip_close(chip);
}

void MotorDriver::startMotor() {
    gpiod_line_request_set_value(request, lf, boolToGpiod(directionLF));
    gpiod_line_request_set_value(request, lb, boolToGpiod(directionLB));
    gpiod_line_request_set_value(request, rf, boolToGpiod(directionRF));
    gpiod_line_request_set_value(request, rb, boolToGpiod(directionRB));
    update();
    PWML.enable(true);
    PWMR.enable(true);
}

void MotorDriver::stopMotor() {
    PWML.enable(false);
    PWMR.enable(false);

    gpiod_line_request_set_value(request, lf, boolToGpiod(false));
    gpiod_line_request_set_value(request, lb, boolToGpiod(false));
    gpiod_line_request_set_value(request, rf, boolToGpiod(false));
    gpiod_line_request_set_value(request, rb, boolToGpiod(false));
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
    gpiod_line_request_set_value(request, lf, boolToGpiod(directionLF));
    gpiod_line_request_set_value(request, lb, boolToGpiod(directionLB));
    gpiod_line_request_set_value(request, rf, boolToGpiod(directionRF));
    gpiod_line_request_set_value(request, rb, boolToGpiod(directionRB));
}

gpiod_line_value MotorDriver::boolToGpiod(bool value) {
    if (value) {
        return GPIOD_LINE_VALUE_ACTIVE;
    }
    return GPIOD_LINE_VALUE_INACTIVE;
}