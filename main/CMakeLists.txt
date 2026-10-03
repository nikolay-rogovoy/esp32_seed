idf_component_register(
    SRCS "main.c" "wifi.c" "http_server.c"
    INCLUDE_DIRS "."
    EMBED_TXTFILES "www/index.html"
    PRIV_REQUIRES
        nvs_flash
        esp_event
        esp_netif
        esp_wifi
        esp_http_server
        esp_driver_gpio
)
