#pragma once
#include "controller.hpp"
#include "esp_log.h"
#include <optional>
#include <memory>

struct request_data
{
    std::optional<RgbColor> color;
    std::optional<uint8_t> brightness;
    std::optional<Effect> effect;
    std::optional<uint8_t> effectSpeed;
};

request_data default_request_data() {
    return request_data {
        .color = std::nullopt,
        .brightness = std::nullopt,
        .effect = std::nullopt,
        .effectSpeed = std::nullopt
    };
}

class Server
{
public:
    static const char *TAG;
    Server(Controller& ctrlPtr);
    ~Server();
    httpd_handle_t start();
    void stop();
    bool isReady();

private:
    Controller& controller;
    httpd_handle_t server = NULL;

    esp_err_t mount_storage();

    static esp_err_t landing_page_handler(httpd_req_t *req);
    static esp_err_t status_handler(httpd_req_t *req);
    static esp_err_t color_handler(httpd_req_t *req);

    httpd_uri_t landing_page = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = landing_page_handler,
        .user_ctx = this
        };

    httpd_uri_t status = {
        .uri = "/status",
        .method = HTTP_GET,
        .handler = status_handler,
        .user_ctx = this
        };

    httpd_uri_t color = {
        .uri = "/color",
        .method = HTTP_POST,
        .handler = color_handler,
        .user_ctx = this
        };
};