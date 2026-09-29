/**
 * @file protocol.hpp
 * @brief Binary wire protocol for commands sent from the ground controller.
 */
#pragma once
#include <bit>
#include <cstdint>
#include <cstring>
#include <optional>
#include <span>
#include <type_traits>

namespace c3copter::protocol {

// The packet is memcpy'd straight from the network buffer, so the host byte
// order must match the wire format. ESP32-C3 (RISC-V) is little-endian.
static_assert(std::endian::native == std::endian::little);

#pragma pack(push, 1)
/**
 * @brief Control command packet (14 bytes on the wire).
 */
struct CommandPacket {
    static constexpr uint16_t kMagic = 0xC3C0;

    uint16_t magic;
    uint16_t seq;
    int16_t  roll, pitch, yaw;
    uint16_t throttle;
    uint8_t  flags;
    uint8_t  crc8;
};
#pragma pack(pop)

static_assert(sizeof(CommandPacket) == 14, "CommandPacket size must be exactly 14 bytes!");
// Required for the memcpy-based parsing in parse().
static_assert(std::is_trivially_copyable_v<CommandPacket>);

[[nodiscard]] constexpr uint8_t crc8(std::span<const std::byte> data) noexcept {
    uint8_t crc = 0;
    for (auto b : data) {
        crc ^= static_cast<uint8_t>(b);
        for (int i = 0; i < 8; ++i)
            crc = (crc & 0x80) ? uint8_t((crc << 1) ^ 0x07) : uint8_t(crc << 1);
    }
    return crc;
}

[[nodiscard]] inline std::optional<CommandPacket> parse(std::span<const std::byte> raw) noexcept {
    if (raw.size() != sizeof(CommandPacket)) return std::nullopt;

    CommandPacket p;
    std::memcpy(&p, raw.data(), sizeof p);

    if (p.magic != CommandPacket::kMagic) return std::nullopt;
    if (crc8(raw.first(sizeof p - 1)) != p.crc8) return std::nullopt;
    return p;
}

[[nodiscard]] constexpr bool is_newer(uint16_t a, uint16_t b) noexcept {
    return static_cast<int16_t>(a - b) > 0;
}

} // namespace c3copter::protocol