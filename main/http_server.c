#include "http_server.h"

#include "esp_log.h"

static const char *TAG = "http";

// www/index.html встраивается в прошивку через EMBED_TXTFILES (см. main/CMakeLists.txt).
// Для TXT-файлов в конец добавляется '\0', поэтому страницу можно отдавать как строку.
extern const char index_html_start[] asm("_binary_index_html_start");

static esp_err_t index_get_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    return httpd_resp_sendstr(req, index_html_start);
}

static const httpd_uri_t index_uri = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = index_get_handler,
};

httpd_handle_t http_server_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(httpd_start(&server, &config));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &index_uri));

    ESP_LOGI(TAG, "HTTP server started on port %d", config.server_port);
    return server;
}
