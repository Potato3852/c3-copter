#pragma once

#include <span>
#include <cstdint>
#include "driver/spi_master.h"
#include "driver/gpio.h"

namespace hal {

class Esp32Spi {
private:
    spi_host_device_t host_;
    gpio_num_t cs_pin_;
    spi_device_handle_t handle_{nullptr};
    
public:
    Esp32Spi(spi_host_device_t host, gpio_num_t cs_pin)
        : host_(host), cs_pin_(cs_pin) {}

    [[nodiscard]] bool init(int clock_speed_hz = 10'000'000) noexcept {
        gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << cs_pin_),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE
        };
        if (gpio_config(&io_conf) != ESP_OK) return false;
        
        deselect();

        spi_device_interface_config_t devcfg = {
            .mode = 0,
            .clock_speed_hz = clock_speed_hz,
            .spics_io_num = -1,
            .queue_size = 1
        };

        return spi_bus_add_device(host_, &devcfg, &handle_) == ESP_OK;
    }

    void select() noexcept {
        gpio_set_level(cs_pin_, 0);
    }

    void deselect() noexcept {
        gpio_set_level(cs_pin_, 1);
    }

    [[nodiscard]] bool transfer(std::span<const uint8_t> tx_buf, std::span<uint8_t> rx_buf) noexcept {
        spi_transaction_t t{};
        t.length = tx_buf.size() * 8;
        t.tx_buffer = tx_buf.data();
        t.rx_buffer = rx_buf.data();

        return spi_device_polling_transmit(handle_, &t) == ESP_OK;
    }
};

} // namespace hal