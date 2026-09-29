/**
 * @file udp.hpp
 * @brief UDP receiver task that turns incoming datagrams into commands.
 */
#pragma once

#include <atomic>
#include <cstdint>
#include "mailbox.hpp"
#include "protocol.hpp"
#include "esp_err.h"

namespace c3copter::telemetry {

/**
 * @brief A command together with its reception timestamp.
 *
 * The timestamp lets the consumer detect a lost link: peek() keeps returning
 * the last command forever, so the age must be checked (failsafe).
 */
struct StampedCommand {
    protocol::CommandPacket cmd;
    int64_t rt_time_us;
};

/**
 * @brief Receives command datagrams over UDP and publishes the newest one.
 */
class UdpCommandReceiver {
private:
    /** @brief FreeRTOS entry point trampoline; @p self is the receiver instance. */
    static void task_entry(void* self) { static_cast<UdpCommandReceiver*>(self)->run(); }

    /** @brief Task body: opens the socket and loops forever receiving packets. */
    void run();

    Mailbox<StampedCommand>& out_;
    uint16_t port_;
    std::atomic<uint32_t> dropped_{0};

public:
    UdpCommandReceiver(Mailbox<StampedCommand>& out, uint16_t port)
        : out_{out}, port_{port} {}

    esp_err_t start();

    /** @brief Number of rejected datagrams (malformed, bad CRC, stale or duplicate seq). */
    [[nodiscard]] uint32_t dropped() const noexcept {return dropped_.load(std::memory_order_relaxed); }
};

} // namespace c3copter::telemetry