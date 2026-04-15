//
// Created by cristi on 28.01.2026.
//

#ifndef LICENTA_GPIOCHIP_H
#define LICENTA_GPIOCHIP_H

#include <gpiod.hpp>

class GpioChip {
public:
    static gpiod::chip &Instance();
    static void ResetInstance();
private:
    GpioChip();
    gpiod::chip chip;

    static  GpioChip *instance;
};


#endif //LICENTA_GPIOCHIP_H