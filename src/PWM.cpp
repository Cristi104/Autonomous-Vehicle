//
// Created by cristi on 22.01.2026.
//
#include "../Include/PWM.h"

#include <fstream>
#include <unistd.h>

PWM::PWM(int chip, int channel)
    : chip(chip), channel(channel)
{
    base = "/sys/class/pwm/pwmchip" + std::to_string(chip);
    path = base + "/pwm" + std::to_string(channel);

    write(base + "/export", channel);
    usleep(100000);
    setPeriod(100000);
    // enable(true);
}
void PWM::setPeriod(int period_ns) {
    write(path + "/period", period_ns);
}

void PWM::setDuty(int duty_ns) {
    write(path + "/duty_cycle", duty_ns);
}

void PWM::enable(bool en) {
    write(path + "/enable", en ? 1 : 0);
}

PWM::~PWM() {
    write(base + "/unexport", channel);
}

void PWM::write(const std::string& file, int value) {
    std::ofstream fs(file);
    fs << value;
}
