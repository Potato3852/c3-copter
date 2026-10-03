#pragma once

#include <cstdint>

namespace imu {

namespace reg {
    constexpr uint8_t CONFIG         = 0x1A;
    constexpr uint8_t GYRO_CONFIG    = 0x1B;
    constexpr uint8_t ACCEL_CONFIG   = 0x1C;
    constexpr uint8_t ACCEL_CONFIG_2 = 0x1D;
    constexpr uint8_t PWR_MGMT_1     = 0x6B;
    constexpr uint8_t WHO_AM_I       = 0x75;
} // namespace imu::reg

constexpr uint8_t MPU9250_ID = 0x71;
constexpr uint8_t MPU6500_ID = 0x70;

static constexpr float GYRO_SCALE  = 1.0f / 16.4f;
static constexpr float ACCEL_SCALE = 1.0f / 4096.0f;

struct RawImuFrame {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t temp;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
};

struct Vector3f {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

struct ImuData {
    Vector3f accel;
    Vector3f gyro;
    float    temp_c;
    uint64_t timestamp;
};

} // namespace imu