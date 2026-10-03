#pragma once

/**
 * Запускает Wi-Fi.
 * Если в menuconfig задан SSID — подключается к сети как станция (STA),
 * иначе поднимает собственную точку доступа (SoftAP).
 */
void wifi_start(void);
