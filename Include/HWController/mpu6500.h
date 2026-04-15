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
    static MPU6500 *Instance();
    static void ResetInstance();
    MPU6500(const MPU6500 &other) = delete;
    MPU6500 &operator=(const MPU6500 &other) = delete;
    MPU6500(MPU6500 &&other) = delete;
    MPU6500 &operator=(MPU6500 &&other) = delete;
    ~MPU6500();
    [[nodiscard]] std::array<float, 3> readAccelerometer() const;
    [[nodiscard]] std::array<float, 3> readGyro() const;

    void test();
    void update();
    void debugLog() const;
    std::array<float, 3> speed;
    std::array<float, 3> angle;
private:
    MPU6500();
    int MPUFD;
    std::array<float, 3> accelOffset;
    std::array<float, 3> gyroOffset;
    std::array<float, 3> position;

    void writeRegister(uint8_t reg, uint8_t value) const;
    [[nodiscard]] uint8_t readRegister(uint8_t reg) const;
    [[nodiscard]] std::array<float, 3> readRawAccelerometer() const;
    [[nodiscard]] std::array<float, 3> readRawGyro() const;

    static MPU6500 *instance;
};


#endif //LICENTA_MPU6500_H