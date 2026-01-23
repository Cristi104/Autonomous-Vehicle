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

#include <cmath>

#define DEV_FILE "/dev/i2c-1"

void MPU6500::writeRegister(uint8_t reg, uint8_t value) const {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    long err = write(MPUFD, buf, 2);
    if (err < 0) {
        std::cerr << "write on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
    }
}

uint8_t MPU6500::readRegister(uint8_t reg) const {
    long err = write(MPUFD, &reg, 1);
    if (err < 0) {
        std::cerr << "write on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
        return 0;
    }
    uint8_t buf = 0;
    err = read(MPUFD, &buf, 1);
    if (err < 0) {
        std::cerr << "read on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
    }
    return buf;
}

std::array<float, 3> MPU6500::readRawAccelerometer() const {
    uint8_t accel[6];
    uint8_t reg = 0x3B;
    long err = write(MPUFD, &reg, 1);
    if (err < 0) {
        std::cerr << "write on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
        return {0};
    }
    err = read(MPUFD, &accel, 6);
    if (err < 0) {
        std::cerr << "read on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
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
        std::cerr << "write on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
        return {0};
    }
    err = read(MPUFD, &gyro, 6);
    if (err < 0) {
        std::cerr << "read on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
    }
    return {
        static_cast<float>(static_cast<int16_t>((gyro[0] << 8) | gyro[1]) / 131.0),
        static_cast<float>(static_cast<int16_t>((gyro[2] << 8) | gyro[3]) / 131.0),
        static_cast<float>(static_cast<int16_t>((gyro[4] << 8) | gyro[5]) / 131.0)
    };
}

MPU6500::MPU6500() :  accelOffset({0, 0, 0}), gyroOffset({0, 0, 0}), speed({0, 0, 0}), position({0, 0, 0}), angle({0, 0, 0}) {
    MPUFD = open(DEV_FILE, O_RDWR);
    if (MPUFD < 0) {
        std::cerr << "open on " << DEV_FILE << " failed: " << std::strerror(errno) << std::endl;
    }
    int err = ioctl(MPUFD, I2C_SLAVE, 0x68);
    if (err < 0) {
        std::cerr << "ioctl on " << MPUFD << " failed: " << std::strerror(errno) << std::endl;
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

MPU6500::~MPU6500() {
    close(MPUFD);
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

void MPU6500::test() {
    for (int i = 0; i < 1000; i++) {
        update();
        debugLog();
        // std::array<float, 3> acceleration = readAccelerometer();
        // std::array<float, 3> gyro = readGyro();
        // std::cout << "acceleration: " << acceleration[0] << " " << acceleration[1] << " " << acceleration[2] << " gyro: " << gyro[0] << " " << gyro[1] << " " << gyro[2] << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void MPU6500::update() {
    static auto lastUpdateTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();

    long deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime).count();
    lastUpdateTime = currentTime;

    std::array<float, 3> acceleration = readAccelerometer();
    acceleration[0] *= 9.80665f;
    acceleration[1] *= 9.80665f;
    acceleration[2] *= 9.80665f;
    std::array<float, 3> gyro = readGyro();
    for (int i = 0; i < 3; i++) {
        if (std::abs(gyro[i]) < 0.2f) {
            gyro[i] = 0.0f;
        }
    }
    gyro[0] *= 0.01745;
    gyro[1] *= 0.01745;
    gyro[2] *= 0.01745;

    float t = deltaTime / 1000.0;

    angle[0] += gyro[0] * t;
    angle[1] += gyro[1] * t;
    angle[2] += gyro[2] * t;

    // float cx = std::cos(angle[0]), sx = std::sin(angle[0]);
    // float cy = std::cos(angle[1]), sy = std::sin(angle[1]);
    float cz = std::cos(angle[2]), sz = std::sin(angle[2]);
    // float M[3][3] = {
    //     {cy * cz, cy * sz, -sy},
    //     {sx * sy * cz - cx * sz, sx * sy * sz + cx * cz, sx * cy},
    //     {cx * sy * cz + sx * sz, cx * sy * sz - sx * cz, cx * cy}
    // };

    float M[3][3] = {
        {cz, -sz, 0},
        {sz, cz, 0},
        {0, 0, 1}
    };

    std::array<float, 3> rotatedAcceleration = acceleration;
    // std::array<float, 3> rotatedAcceleration = {
    //     acceleration[0] * M[0][0] + acceleration[1] * M[0][1] + acceleration[2] * M[0][2],
    //     acceleration[0] * M[1][0] + acceleration[1] * M[1][1] + acceleration[2] * M[1][2],
    //     acceleration[0] * M[2][0] + acceleration[1] * M[2][1] + acceleration[2] * M[2][2]
    // };
    for (int i = 0; i < 3; i++) {
        if (std::abs(rotatedAcceleration[i]) < 0.1f) {
            rotatedAcceleration[i] = 0.0f;
        }
    }

    float accelMag = sqrt(rotatedAcceleration[0] * rotatedAcceleration[0] + rotatedAcceleration[1] * rotatedAcceleration[1] + rotatedAcceleration[2] * rotatedAcceleration[2]);
    float gyroMag  = sqrt(gyro[0] * gyro[0] + gyro[1] * gyro[1] + gyro[2] * gyro[2]);

    bool stationary =
        std::abs(accelMag) < 0.1f &&
        gyroMag < 0.02f;

    // std::cout << stationary << ' ' << accelMag << ' ' << gyroMag << "acceleration: " << acceleration[0] << " " << acceleration[1] << " " << acceleration[2] << " gyro: " << gyro[0] << " " << gyro[1] << " " << gyro[2] << std::endl;
    if (stationary) {
        speed = {0, 0, 0};
    }

    float maxSpeed = 4.0f;
    for (int i = 0; i < 3; i++) {
        speed[i] = std::clamp(speed[i], -maxSpeed, maxSpeed);
    }

    position[0] += speed[0] * t;
    position[1] += speed[1] * t;
    position[2] += speed[2] * t;

    speed[0] += rotatedAcceleration[0] * t;
    speed[1] += rotatedAcceleration[1] * t;
    speed[2] += rotatedAcceleration[2] * t;
}

void MPU6500::debugLog() const {
    std::cout << std::setprecision(4) << std::fixed;
    std::cout << "speed: " << speed[0] << " " << speed[1] << " " << speed[2]
    // << " position: " << position[0] << " " << position[1] << " " << position[2]
    << " angle: " << angle[0] / 0.01745 << " " << angle[1] / 0.01745 << " " << angle[2] / 0.01745
    << std::endl;
}
