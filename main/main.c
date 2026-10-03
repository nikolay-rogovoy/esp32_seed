#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "sdkconfig.h"

#include "http_server.h"
#include "wifi.h"

static const char *TAG = "main";

#if CONFIG_APP_LED_GPIO >= 0
static void blink_task(void *arg)
{
    const gpio_num_t led = (gpio_num_t)CONFIG_APP_LED_GPIO;
    bool on = false;

    gpio_reset_pin(led);
    gpio_set_direction(led, GPIO_MODE_OUTPUT);

    for (;;) {
        on = !on;
        gpio_set_level(led, on);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
#endif

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 seed starting");

    // NVS нужен драйверу Wi-Fi для хранения калибровки
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

#if CONFIG_APP_LED_GPIO >= 0
    xTaskCreate(blink_task, "blink", 2048, NULL, 5, NULL);
#endif

    wifi_start();
    http_server_start();
}
