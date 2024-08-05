#pragma once
#include "controller.hpp"
#include "esp_log.h"
#include <optional>

struct request_data
{
    std::optional<RgbColor> color;
    std::optional<uint8_t> brightness;
    std::optional<Effect> effect;
    std::optional<uint8_t> effectSpeed;
};

class Server
{
public:
    static const char *TAG;
    Server(Controller *controller);
    ~Server();
    httpd_handle_t start();
    void stop();
    bool isReady();

private:
    Controller *controller;
    httpd_handle_t server = NULL;

    static esp_err_t status_handler(httpd_req_t *req);
    static esp_err_t color_handler(httpd_req_t *req);
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