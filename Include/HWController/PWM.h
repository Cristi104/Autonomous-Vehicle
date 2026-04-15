//
// Created by cristi on 22.01.2026.
//

#ifndef LICENTA_PWM_H
#define LICENTA_PWM_H

#include <string>

class PWM {
public:
    PWM(int chip, int channel);
    PWM(const PWM &other) = delete;
    PWM &operator=(const PWM &other) = delete;
    // TODO: build actual move constructor
    PWM(PWM &&other) = delete;
    PWM &operator=(PWM &&other) = delete;
    ~PWM();

    void setPeriod(int period_ns);
    void setDuty(int duty_ns);
    void enable(bool en);

private:
    int chip;
    int channel;
    std::string base;
    std::string path;

    void write(const std::string& file, int value);
};

#endif //LICENTA_PWM_H