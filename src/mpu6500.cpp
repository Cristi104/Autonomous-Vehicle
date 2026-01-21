#include <cstring>
#include <iostream>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <cstdint>
#include <unistd.h>
#include <fcntl.h>
#include <chrono>
#include <thread>
#include "../Include/mpu6500.h"

#define DEV_FILE "/dev/i2c-1"

void MPU6500::writeRegister(uint8_t reg, uint8_t value) const {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    long err = write(MPUFD, buf, 2);
    if (err < 0) {
        std::cout << "write on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
    }
}

uint8_t MPU6500::readRegister(uint8_t reg) const {
    long err = write(MPUFD, &reg, 1);
    if (err < 0) {
        std::cout << "write on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
        return 0;
    }
    uint8_t buf = 0;
    err = read(MPUFD, &buf, 1);
    if (err < 0) {
        std::cout << "read on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
    }
    return buf;
}

std::array<float, 3> MPU6500::readRawAccelerometer() const {
    uint8_t accel[6];
    uint8_t reg = 0x3B;
    long err = write(MPUFD, &reg, 1);
    if (err < 0) {
        std::cout << "write on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
        return {0};
    }
    err = read(MPUFD, &accel, 6);
    if (err < 0) {
        std::cout << "read on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
    }
    return {
        static_cast<float>(static_cast<int16_t>((accel[0] << 8) | accel[1]) / 8192.0),
        static_cast<float>(static_cast<int16_t>((accel[2] << 8) | accel[3]) / 8192.0),
        static_cast<float>(static_cast<int16_t>((accel[4] << 8) | accel[5]) / 8192.0)
    };
}

std::array<float, 3> MPU6500::readRawGyro() const {
    uint8_t gyro[6];
    uint8_t reg = 0x43;
    long err = write(MPUFD, &reg, 1);
    if (err < 0) {
        std::cout << "write on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
        return {0};
    }
    err = read(MPUFD, &gyro, 6);
    if (err < 0) {
        std::cout << "read on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
    }
    return {
        static_cast<float>(static_cast<int16_t>((gyro[0] << 8) | gyro[1]) / 131.0),
        static_cast<float>(static_cast<int16_t>((gyro[2] << 8) | gyro[3]) / 131.0),
        static_cast<float>(static_cast<int16_t>((gyro[4] << 8) | gyro[5]) / 131.0)
    };
}

MPU6500::MPU6500() :  accelOffset({0, 0, 0}), gyroOffset({0, 0, 0}) {
    MPUFD = open(DEV_FILE, O_RDWR);
    if (MPUFD < 0) {
        std::cout << "open on " << DEV_FILE << " failed: " << std::strerror(errno) << std::endl;
    }
    int err = ioctl(MPUFD, I2C_SLAVE, 0x68);
    if (err < 0) {
        std::cout << "ioctl on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
        close(MPUFD);
    }
    // reset mpu to default settings
    writeRegister(0x6B, 0x80);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    writeRegister(0x6B, 0x01);

    // set gyro precision to 250deg/s
    writeRegister(0x1B, 0x00);

    // set accelerometer precision to 4g
    writeRegister(0x1C, 0x08);

    // set low pass filter on
    writeRegister(0x1A, 0x03);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // read offests
    int samples = 50;
    for (int i = 0; i < samples; i++) {
        std::array<float, 3> acceleration = readRawAccelerometer();
        accelOffset[0] += acceleration[0];
        accelOffset[1] += acceleration[1];
        accelOffset[2] += acceleration[2];

        std::array<float, 3> gyro = readRawGyro();
        gyroOffset[0] += gyro[0];
        gyroOffset[1] += gyro[1];
        gyroOffset[2] += gyro[2];
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    accelOffset[0] /= samples;
    accelOffset[1] /= samples;
    accelOffset[2] /= samples;

    gyroOffset[0] /= samples;
    gyroOffset[1] /= samples;
    gyroOffset[2] /= samples;
}

std::array<float, 3> MPU6500::readAccelerometer() const {
    std::array<float, 3> acceleration = readRawAccelerometer();
    acceleration[0] -= accelOffset[0];
    acceleration[1] -= accelOffset[1];
    acceleration[2] -= accelOffset[2];
    return acceleration;
}

std::array<float, 3> MPU6500::readGyro() const {
    std::array<float, 3> gyro = readRawGyro();
    gyro[0] -= gyroOffset[0];
    gyro[1] -= gyroOffset[1];
    gyro[2] -= gyroOffset[2];
    return gyro;
}

void MPU6500::test() const {
    for (int i = 0; i < 30; i++) {
        std::array<float, 3> acceleration = readAccelerometer();
        std::array<float, 3> gyro = readGyro();
        std::cout << "acceleration: " << acceleration[0] << " " << acceleration[1] << " " << acceleration[2] << " gyro: " << gyro[0] << " " << gyro[1] << " " << gyro[2] << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
