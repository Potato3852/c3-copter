#include <cstdio>
#include <span>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "wifi_ap.hpp"

static constexpr const char* TAG = "C3_COPTER";

void wifi_monitor_task(void* pvParameters) {
    auto* wifi = static_cast<c3copter::telemetry::WifiAP*>(pvParameters);

    while (true) {
        ESP_LOGI(TAG, "[Wi-Fi Status] Connected clients: %d", wifi->get_client_count());
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

extern "C" void app_main() {
    ESP_LOGI(TAG, "Starting c3-copter firmware...");

    static c3copter::telemetry::WifiAP wifi_ap;
    esp_err_t err = wifi_ap.init("C3-Copter", "12345678");

    if (err == ESP_OK) {
        xTaskCreate(wifi_monitor_task, "wifi_monitor", 2048, &wifi_ap, 1,nullptr);
    } else {
        ESP_LOGE(TAG, "Failed to start Wi-Fi SoftAP!");
    }
}