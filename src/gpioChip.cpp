//
// Created by cristi on 28.01.2026.
//

#include "../Include/gpioChip.h"

GpioChip * GpioChip::instance = nullptr;
gpiod::chip & GpioChip::Instance() {
    if (instance == nullptr) {
        instance = new GpioChip;
    }
    return instance->chip;
}

void GpioChip::ResetInstance() {
    delete instance;
    instance = nullptr;
}

GpioChip::GpioChip() :chip(std::filesystem::path("/dev/gpiochip0")) {}
