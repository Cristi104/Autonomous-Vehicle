//
// Created by cristi on 23.01.2026.
//

#ifndef LICENTA_ULTRASONICDISTANCE_H
#define LICENTA_ULTRASONICDISTANCE_H

#include "gpiod.h"

class UltrasonicDistance {
public:
    UltrasonicDistance(gpiod_chip *chip, unsigned int trig, unsigned int echo);
    void ping();
    bool update();
    float getDistance();
    ~UltrasonicDistance();

private:
    gpiod_chip *chip;
    gpiod_line_request* request;
    gpiod_edge_event_buffer *event;
    unsigned int trig;
    unsigned int echo;
    float distance;
};


#endif //LICENTA_ULTRASONICDISTANCE_H