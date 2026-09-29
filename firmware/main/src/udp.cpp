#include "udp.hpp"
#include <array>
#include <optional>
#include "esp_log.h"
#include "esp_timer.h"
#include "lwip/sockets.h"

namespace c3copter::telemetry {

static constexpr const char* TAG = "UDP_RX";

esp_err_t UdpCommandReceiver::start() {
    BaseType_t ok = xTaskCreate(task_entry, "udp_rx", 4096, this, 10, nullptr);
    return ok == pdPASS ? ESP_OK : ESP_ERR_NO_MEM;
}

void UdpCommandReceiver::run() {
    int sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) { 
        ESP_LOGE(TAG, "socket failed");
        vTaskDelete(nullptr);
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof addr) < 0) {
        ESP_LOGE(TAG, "bind failed");
        ::close(sock);
        vTaskDelete(nullptr);
        return;
    }

    timeval tv{.tv_sec = 0, .tv_usec = 100'000};
    ::setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);

    ESP_LOGI(TAG, "UDP Command Receiver started on port %d", port_);

    std::array<std::byte, 64> buf;
    std::optional<uint16_t> last_seq;

    for (;;) {
        int n = ::recvfrom(sock, buf.data(), buf.size(), 0, nullptr, nullptr);
        if (n <= 0) continue;

        auto pkt = protocol::parse({buf.data(), static_cast<size_t>(n)});
        
        if (!pkt || (last_seq && !protocol::is_newer(pkt->seq, *last_seq))) {
            dropped_.fetch_add(1, std::memory_order_relaxed);
            continue;
        }
        last_seq = pkt->seq;
        out_.post({*pkt, esp_timer_get_time()});
    }
}

} // namespace c3copter::telemetry