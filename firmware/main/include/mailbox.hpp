/**
 * @file mailbox.hpp
 * @brief Single-slot, "latest value wins" mailbox for inter-task communication.
 */
#pragma once

#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include <type_traits>

namespace c3copter {

template<typename T>
requires std::is_trivially_copyable_v<T>
class Mailbox{
private:
    alignas(T) uint8_t storage_[sizeof(T)];    
    StaticQueue_t qbuf_;
    QueueHandle_t q_;

public:
    /** @brief Creates the underlying static queue (length 1). Never allocates. */
    Mailbox() noexcept: q_ {xQueueCreateStatic(1, sizeof(T), storage_, &qbuf_)} {}
    ~Mailbox() = default;

    Mailbox(const Mailbox&) = delete;
    Mailbox& operator=(const Mailbox&) = delete;
    Mailbox(Mailbox&&) = delete;
    Mailbox& operator=(Mailbox&&) = delete;

    /**
     * @brief Stores @p v, replacing any previous value. Never blocks.
     * @param v Value to publish.
     */
    void post(const T& v) noexcept { xQueueOverwrite(q_, &v); }

    /**
     * @brief Copies the current value into @p out without removing it.
     * @return true if a value has ever been posted, false if the box is empty.
     */
    [[nodiscard]] bool peek(T& out) const noexcept {
        return xQueuePeek(q_, &out, 0) == pdTRUE;
    }
};

} // namespace c3copter