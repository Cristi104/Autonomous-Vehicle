#include <cstring>
#include <iostream>
#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <chrono>
#include <thread>

static float accel_offset[3] = {0, 0, 0};
static float gyro_offset[3] = {0, 0, 0};

uint8_t read_i2c_byte(int fd, uint8_t reg) {
    uint8_t data = 0;
    int err = write(fd, &reg, 1);
    if (err < 0) {
        printf("write failed: %s\n", strerror(errno));
        close(fd);
    }
    err = read(fd, &data, 1);
    if (err < 0) {
        printf("read failed: %s\n", strerror(errno));
        close(fd);
    }
    return data;
}

void write_i2c_byte(int fd, uint8_t reg, uint8_t data) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = data;
    int err = write(fd, buf, 2);
    if (err < 0) {
        printf("write failed: %s\n", strerror(errno));
        close(fd);
    }
}

void read_i2c(int fd, uint8_t reg, uint8_t *data, uint16_t len) {
    int err = write(fd, &reg, 1);
    if (err < 0) {
        printf("write failed: %s\n", strerror(errno));
        close(fd);
    }
    err = read(fd, data, len);
    if (err < 0) {
        printf("read failed: %s\n", strerror(errno));
        close(fd);
    }
}

void read_raw_mpu(int fd, float* accleration, float* gyro) {
    uint8_t raw[6];
    int16_t buf[3];

    read_i2c(fd, 0x3B, raw, 6);
    // big-endian to litle-endian
    buf[0] = raw[0] << 8 | raw[1];
    buf[1] = raw[2] << 8 | raw[3];
    buf[2] = raw[4] << 8 | raw[5];

    accleration[0] = buf[0] / 8196.0;
    accleration[1] = buf[1] / 8196.0;
    accleration[2] = buf[2] / 8196.0;

    read_i2c(fd, 0x43, raw, 6);
    // big-endian to litle-endian
    buf[0] = raw[0] << 8 | raw[1];
    buf[1] = raw[2] << 8 | raw[3];
    buf[2] = raw[4] << 8 | raw[5];

    gyro[0] = buf[0] / 131.0;
    gyro[1] = buf[1] / 131.0;
    gyro[2] = buf[2] / 131.0;
}

void read_mpu(int fd, float* accleration, float* gyro) {
    read_raw_mpu(fd, accleration, gyro);
    accleration[0] -= accel_offset[0];
    accleration[1] -= accel_offset[1];
    accleration[2] -= accel_offset[2];

    gyro[0] -= gyro_offset[0];
    gyro[1] -= gyro_offset[1];
    gyro[2] -= gyro_offset[2];
}

int init_mpu() {
    int I2C_FD = open("/dev/i2c-1", O_RDWR);
    if (I2C_FD < 0) {
        printf("open failed: %s\n", strerror(errno));
    }
    int err = ioctl(I2C_FD, I2C_SLAVE, 0x68);
    if (err < 0) {
        printf("ioctl failed: %s\n", strerror(errno));
        close(I2C_FD);
    }


    // reset mpu to default settings
    write_i2c_byte(I2C_FD, 0x6B, 0x80);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    write_i2c_byte(I2C_FD, 0x6B, 0x01);

    // set gyro precision to 250deg/s
    write_i2c_byte(I2C_FD, 0x1B, 0x00);

    // set accelerometer precision to 4g/s
    write_i2c_byte(I2C_FD, 0x1C, 0x08);

    // set low pass filter on
    write_i2c_byte(I2C_FD, 0x1A, 0x03);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // read offests
    float accleration[3] = {0, 0, 0};
    float gyro[3] = {0, 0, 0};
    int samples = 50;
    for (int i = 0; i < samples; i++) {
        read_raw_mpu(I2C_FD, accleration, gyro);
        accel_offset[0] += accleration[0];
        accel_offset[1] += accleration[1];
        accel_offset[2] += accleration[2];

        gyro_offset[0] += gyro[0];
        gyro_offset[1] += gyro[1];
        gyro_offset[2] += gyro[2];

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    accel_offset[0] /= samples;
    accel_offset[1] /= samples;
    accel_offset[2] /= samples;

    gyro_offset[0] /= samples;
    gyro_offset[1] /= samples;
    gyro_offset[2] /= samples;

    return I2C_FD;
}

int main() {
    int MPU_FD = init_mpu();
    float accleration[3] = {0, 0, 0};
    float gyro[3] = {0, 0, 0};
    for (int i = 0; i < 30; i++) {
        read_mpu(MPU_FD, accleration, gyro);
        printf("accleration: %f, %f, %f, gyro: %f, %f, %f\n", accleration[0], accleration[1], accleration[2], gyro[0], gyro[1], gyro[2]);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    close(MPU_FD);
    return 0;
}
