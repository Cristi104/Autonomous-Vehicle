//
// Created by cristi on 23.01.2026.
//

#ifndef LICENTA_ULTRASONICDISTANCE_H
#define LICENTA_ULTRASONICDISTANCE_H

#include <gpiod.hpp>

class UltrasonicDistance {
public:
    UltrasonicDistance(gpiod::chip &chip, unsigned int trig, unsigned int echo);
    void ping();
    bool update();
    float getDistance();
    ~UltrasonicDistance() = default;

private:
    gpiod::chip &chip;
    std::optional<gpiod::line_request> request;
    gpiod::edge_event_buffer event;
    unsigned int trig;
    unsigned int echo;
    float distance;
};


#endif //LICENTA_ULTRASONICDISTANCE_H