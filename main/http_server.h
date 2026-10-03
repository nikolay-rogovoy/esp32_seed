#pragma once

#include "esp_http_server.h"

/**
 * Запускает HTTP-сервер на порту 80 и регистрирует обработчики.
 * Сервер работает в собственной задаче FreeRTOS.
 */
httpd_handle_t http_server_start(void);
