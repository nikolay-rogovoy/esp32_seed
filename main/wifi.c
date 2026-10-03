#include "wifi.h"

#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "sdkconfig.h"

static const char *TAG = "wifi";

static void on_wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Disconnected, reconnecting...");
        esp_wifi_connect();
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t *event = (const ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "Connected, open http://" IPSTR "/", IP2STR(&event->ip_info.ip));
    }
}

static void init_driver(void)
{
    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, on_wifi_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, on_wifi_event, NULL));
}

static void start_sta(void)
{
    esp_netif_create_default_wifi_sta();
    init_driver();

    wifi_config_t cfg = {
        .sta = {
            .threshold.authmode = strlen(CONFIG_APP_WIFI_PASSWORD) > 0 ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN,
        },
    };
    strlcpy((char *)cfg.sta.ssid, CONFIG_APP_WIFI_SSID, sizeof(cfg.sta.ssid));
    strlcpy((char *)cfg.sta.password, CONFIG_APP_WIFI_PASSWORD, sizeof(cfg.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to \"%s\"...", CONFIG_APP_WIFI_SSID);
}

static void start_ap(void)
{
    esp_netif_t *netif = esp_netif_create_default_wifi_ap();
    init_driver();

    wifi_config_t cfg = {
        .ap = {
            .channel = 1,
            .max_connection = 4,
            .authmode = strlen(CONFIG_APP_AP_PASSWORD) > 0 ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN,
        },
    };
    strlcpy((char *)cfg.ap.ssid, CONFIG_APP_AP_SSID, sizeof(cfg.ap.ssid));
    cfg.ap.ssid_len = strlen((const char *)cfg.ap.ssid);
    strlcpy((char *)cfg.ap.password, CONFIG_APP_AP_PASSWORD, sizeof(cfg.ap.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &cfg));
    ESP_ERROR_CHECK(esp_wifi_start());

    esp_netif_ip_info_t ip;
    ESP_ERROR_CHECK(esp_netif_get_ip_info(netif, &ip));
    ESP_LOGI(TAG, "SoftAP \"%s\" started, open http://" IPSTR "/", CONFIG_APP_AP_SSID, IP2STR(&ip.ip));
}

void wifi_start(void)
{
    if (strlen(CONFIG_APP_WIFI_SSID) > 0) {
        start_sta();
    } else {
        start_ap();
    }
}
