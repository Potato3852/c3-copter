idf_component_register(
    SRCS 
        "main.cpp"
        "src/wifi_ap.cpp"
        "src/udp.cpp"
    INCLUDE_DIRS 
        "include"
    REQUIRES
        freertos
        driver
        esp_wifi 
        esp_event 
        nvs_flash 
        esp_netif
        esp_timer
)

target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_20)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -fno-exceptions
    -fno-rtti
    -O2
    -Wall
    -Wextra
)