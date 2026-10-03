#pragma once

#include <span>
#include <concepts>
#include <cstdint>
#include <optional>
#include "imu_types.hpp"

namespace imu {

template <typename T>
concept SpiBusConcept = requires(T bus, std::span<const uint8_t> tx_buf, 
                                 std::span<uint8_t> rx_buf) {
    { bus.select() } -> std::same_as<void>;
    { bus.deselect() } -> std::same_as<void>;
    { bus.transfer(rx_buf, tx_buf) } -> std::same_as<bool>;
};

template <SpiBusConcept SpiBus>
class Mpu9250Reader {
private:
    SpiBus& bus_;
    [[nodiscard]] std::optional<uint8_t> read_register(uint8_t reg) noexcept {
        uint8_t tx[2] = { static_cast<uint8_t>(reg | 0x80), 0x00 };
        uint8_t rx[2] = {0};

        bus_.select();
        bool success = bus_.transfer(std::span<const uint8_t>(tx), std::span<uint8_t>(rx));
        bus_.deselect();

        if (success) {
            return rx[1];
        } else {
            return std::nullopt;
        }
    }

    [[nodiscard]] bool write_register(uint8_t reg, uint8_t value) noexcept {
        uint8_t tx[2] = { reg, value };
        uint8_t rx[2] = {0};

        bus_.select();
        bool success = bus_.transfer(std::span<const uint8_t>(tx), std::span<uint8_t>(rx));
        bus_.deselect();

        return success;
    }

     [[nodiscard]] static constexpr int16_t parse_be_int16(uint8_t msb, uint8_t lsb) noexcept {
        return static_cast<int16_t>((static_cast<uint16_t>(msb) << 8) | lsb);
    }

public:
    explicit constexpr Mpu9250Reader(SpiBus& bus) noexcept : bus_(bus) {}

    [[nodiscard]] bool init() noexcept {
        const auto whoami = read_register(reg::WHO_AM_I);
        if (!whoami.has_value()) return false;
        if (*whoami != MPU9250_ID && *whoami != MPU6500_ID) return false;

        write_register(reg::PWR_MGMT_1, 0x80);
        if (!write_register(reg::PWR_MGMT_1, 0x80)) return false;
        delay_ms_fn(100);

        if (!write_register(reg::PWR_MGMT_1, 0x01)) return false;
        if (!write_register(reg::GYRO_CONFIG, 0x18)) return false;
        if (!write_register(reg::ACCEL_CONFIG, 0x10)) return false;
        if (!write_register(reg::CONFIG, 0x02)) return false;
        if (!write_register(reg::ACCEL_CONFIG_2, 0x02)) return false;

        return true;
    }

    [[nodiscard]] std::optional<RawImuFrame> read_raw() noexcept {
        uint8_t tx[15] = { static_cast<uint8_t>(reg::ACCEL_XOUT_H | 0x80) };
        uint8_t rx[15] = {0};

        bus_.select();
        const bool success = bus_.transfer(std::span<const uint8_t>(tx), std::span<uint8_t>(rx));
        bus_.deselect();

        if (!success) [[unlikely]] {
            return std::nullopt;
        }

        return RawImuFrame{
            .accel_x = parse_be_int16(rx[1],  rx[2]),
            .accel_y = parse_be_int16(rx[3],  rx[4]),
            .accel_z = parse_be_int16(rx[5],  rx[6]),
            .temp    = parse_be_int16(rx[7],  rx[8]),
            .gyro_x  = parse_be_int16(rx[9],  rx[10]),
            .gyro_y  = parse_be_int16(rx[11], rx[12]),
            .gyro_z  = parse_be_int16(rx[13], rx[14])
        };
    }

    [[nodiscard]] std::optional<ImuData> read(uint64_t timestamp_us) noexcept {
        const auto frame = read_raw();

        if (!frame) [[unlikely]] {
            return std::nullopt;
        }

        return ImuData{
            .accel = {
                .x = static_cast<float>(frame->accel_x) * ACCEL_SCALE,
                .y = static_cast<float>(frame->accel_y) * ACCEL_SCALE,
                .z = static_cast<float>(frame->accel_z) * ACCEL_SCALE
            },
            .gyro = {
                .x = static_cast<float>(frame->gyro_x) * GYRO_SCALE,
                .y = static_cast<float>(frame->gyro_y) * GYRO_SCALE,
                .z = static_cast<float>(frame->gyro_z) * GYRO_SCALE
            },
            .temp_c = (static_cast<float>(frame->temp) - 21.0f) / 333.87f + 21.0f,
            .timestamp = timestamp_us
        };
    }
};

} // namespace imu