/**
 * @file wifi_ap.hpp
 * @brief Wi-Fi SoftAP used as the telemetry/control link.
 */
#pragma once

#include <cstdint>
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"

namespace c3copter::telemetry {

/**
 * @brief Brings up the ESP32 as a Wi-Fi access point (192.168.4.1) and tracks
 *        the number of connected stations.
*/
class WifiAP {
private:
    /** @brief ESP event loop callback for WIFI_EVENT (station connect/disconnect). */
    static void event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) noexcept;

    static inline uint8_t client_count_{0};
    static inline const char* TAG{"WIFI_AP"};

public:
    WifiAP() = default;

    esp_err_t init(const char* ssid, const char* password, uint8_t max_connections = 3);

    /** @brief Number of currently connected stations. */
    [[nodiscard]] uint8_t get_client_count() const { return client_count_; }
};

}