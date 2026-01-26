//
// Created by cristi on 23.01.2026.
//

#include "../Include/ultrasonicDistance.h"

#include <chrono>
#include <unistd.h>

UltrasonicDistance::UltrasonicDistance(gpiod::chip &chip, unsigned int trig, unsigned int echo)
    : chip(chip), trig(trig), echo(echo), distance(0), event(1) {
    gpiod::line_settings outSettings;
    outSettings.set_direction(gpiod::line::direction::OUTPUT);
    outSettings.set_output_value(gpiod::line::value::INACTIVE);
    gpiod::line_settings inSettings;
    inSettings.set_direction(gpiod::line::direction::INPUT);
    inSettings.set_edge_detection(gpiod::line::edge::BOTH);

    gpiod::line_config config;
    config.add_line_settings(trig, outSettings);
    config.add_line_settings(echo, inSettings);

    gpiod::request_config req_cfg;
    req_cfg.set_consumer("UltrasoincDistance");

    gpiod::request_builder builder = chip.prepare_request();
    builder.set_request_config(req_cfg);
    builder.set_line_config(config);
    request = builder.do_request();
}

void UltrasonicDistance::ping() {
    request->set_value(trig, gpiod::line::value::ACTIVE);
    usleep(10);
    request->set_value(trig, gpiod::line::value::INACTIVE);
}

bool UltrasonicDistance::update() {
    bool available = request->wait_edge_events(std::chrono::nanoseconds::zero());
    if (!available) {
        return false;
    }
    int ret = request->read_edge_events(event);
    if (ret != 1)
        return false;

    static long startTime = 0;
    if (event.begin()[0].type() == gpiod::edge_event::event_type::RISING_EDGE) {
        startTime = event.begin()[0].timestamp_ns();
        return false;
    } else if (event.begin()[0].type() == gpiod::edge_event::event_type::FALLING_EDGE) {
        long endTime = event.begin()[0].timestamp_ns();
        long deltaTime = endTime - startTime;
        distance = deltaTime * 0.034 / 2 / 1000;
        return true;
    }
    return false;
}

float UltrasonicDistance::getDistance() {
    return distance;
}
