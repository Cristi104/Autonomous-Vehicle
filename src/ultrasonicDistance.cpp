//
// Created by cristi on 23.01.2026.
//

#include "../Include/ultrasonicDistance.h"

#include <chrono>
#include <unistd.h>

UltrasonicDistance::UltrasonicDistance(gpiod_chip *chip, unsigned int trig, unsigned int echo)
    :chip(chip), trig(trig), echo(echo), distance(0){
    gpiod_line_settings* outSettings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(outSettings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(outSettings, GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_settings* inSettings = gpiod_line_settings_new();
    gpiod_line_settings_set_direction(inSettings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(inSettings, GPIOD_LINE_EDGE_BOTH);

    gpiod_line_config* config = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(config, &trig, 1, outSettings);
    gpiod_line_config_add_line_settings(config, &echo, 1, inSettings);

    gpiod_request_config* req_cfg = gpiod_request_config_new();
    gpiod_request_config_set_consumer(req_cfg, "UltrasoincDistance");

    request = gpiod_chip_request_lines(chip, req_cfg, config);

    event = gpiod_edge_event_buffer_new(1);
}

void UltrasonicDistance::ping() {
    gpiod_line_request_set_value(request, trig, GPIOD_LINE_VALUE_ACTIVE);
    usleep(10);
    gpiod_line_request_set_value(request, trig, GPIOD_LINE_VALUE_INACTIVE);
}

bool UltrasonicDistance::update() {
    bool available = gpiod_line_request_wait_edge_events(request, 0);
    if (!available) {
        return false;
    }
    int ret = gpiod_line_request_read_edge_events(request, event, 1);
    if (ret <= 0)
        return false;

    static long startTime = 0;
    gpiod_edge_event_type type = gpiod_edge_event_get_event_type(gpiod_edge_event_buffer_get_event(event, 0));
    if (type == GPIOD_EDGE_EVENT_RISING_EDGE) {
        startTime = gpiod_edge_event_get_timestamp_ns(gpiod_edge_event_buffer_get_event(event, 0));
        return false;
    } else if (type == GPIOD_EDGE_EVENT_FALLING_EDGE) {
        long endTime = gpiod_edge_event_get_timestamp_ns(gpiod_edge_event_buffer_get_event(event, 0));
        long deltaTime = endTime - startTime;
        distance = deltaTime * 0.034 / 2 / 1000;
        return true;
    }
    return false;
}

float UltrasonicDistance::getDistance() {
    return distance;
}

UltrasonicDistance::~UltrasonicDistance() {
    if (request)
        gpiod_line_request_release(request);
    if (event)
        gpiod_edge_event_buffer_free(event);
}


