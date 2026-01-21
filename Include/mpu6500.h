//
// Created by cristi on 22.01.2026.
//

#ifndef LICENTA_MPU6500_H
#define LICENTA_MPU6500_H
#include <array>
#include <fstream>
#include <cstdint>

class MPU6500 {
public:
    MPU6500();
    ~MPU6500() = default;
    std::array<float, 3> readAccelerometer() const;
    std::array<float, 3> readGyro() const;

    void test() const;
private:
    int MPUFD;
    std::array<float, 3> accelOffset;
    std::array<float, 3> gyroOffset;

    void writeRegister(uint8_t reg, uint8_t value) const;
    [[nodiscard]] uint8_t readRegister(uint8_t reg) const;
    [[nodiscard]] std::array<float, 3> readRawAccelerometer() const;
    [[nodiscard]] std::array<float, 3> readRawGyro() const;
};


#endif //LICENTA_MPU6500_H