#include "wifi_ap.hpp"
#include "esp_log.h"
#include "esp_mac.h"
#include <cstring>

namespace c3copter::telemetry {

void WifiAP::event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) noexcept {
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_AP_STACONNECTED: {
                auto* event = static_cast<wifi_event_ap_staconnected_t*>(event_data);
                client_count_++;
                ESP_LOGI(TAG, "Client connected! MAC: %02x:%02x:%02x:%02x:%02x:%02x, AID=%d, Total clients: %d",
                         event->mac[0], event->mac[1], event->mac[2],
                         event->mac[3], event->mac[4], event->mac[5],
                         event->aid, client_count_);
                break;
            }
            case WIFI_EVENT_AP_STADISCONNECTED: {
                auto* event = static_cast<wifi_event_ap_stadisconnected_t*>(event_data);
                if (client_count_ > 0) client_count_--;
                ESP_LOGW(TAG, "Client disconnected! MAC: %02x:%02x:%02x:%02x:%02x:%02x, AID=%d, Total clients: %d",
                         event->mac[0], event->mac[1], event->mac[2],
                         event->mac[3], event->mac[4], event->mac[5],
                         event->aid, client_count_);
                break;
            }
            default:
                break;
        }
    }
}

esp_err_t WifiAP::init(const char* ssid, const char* password, uint8_t max_connections) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, 
        ESP_EVENT_ANY_ID, 
        &WifiAP::event_handler, 
        nullptr, 
        nullptr
    ));

    wifi_config_t wifi_config = {};
    snprintf(reinterpret_cast<char*>(wifi_config.ap.ssid), sizeof(wifi_config.ap.ssid), "%s", ssid);
                  
    wifi_config.ap.ssid_len = static_cast<uint8_t>(std::strlen(reinterpret_cast<char*>(wifi_config.ap.ssid)));
    wifi_config.ap.channel = 6;
    wifi_config.ap.max_connection = max_connections;
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_WPA3_PSK;

    wifi_config.ap.ssid_hidden = 0;

    if (std::strlen(password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    } else {
        snprintf(reinterpret_cast<char*>(wifi_config.ap.password), sizeof(wifi_config.ap.password), "%s", password);
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(40));

    ESP_LOGI(TAG, "SoftAP initialized! SSID: '%s' | IP: 192.168.4.1", ssid);
    return ESP_OK;
}

} // namespace c3copter::telemetry