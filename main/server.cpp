#include "esp_log.h"
#include "freertos/FreeRTOSConfig.h"
#include "esp_http_server.h"
#include "esp_event.h"
#include <sys/param.h>
#include "cJSON.h"
#include "server.hpp"

const char *Server::TAG = "Server";

Server::Server(Controller& ctrlPtr) : controller(ctrlPtr)
{
    server = start();
}

Server::~Server()
{
    stop();
}

httpd_handle_t Server::start()
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(Server::TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK)
    {
        ESP_LOGI(Server::TAG, "Error starting server!");
        return NULL;
    }

    // Set URI handlers
    ESP_LOGI(Server::TAG, "Registering URI handlers");
    // ESP_ERROR_CHECK(httpd_register_uri_handler(server, &status));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &color));
    return server;
}

void Server::stop()
{
    ESP_ERROR_CHECK(httpd_stop(server));
}

/* An HTTP GET handler */
esp_err_t Server::status_handler(httpd_req_t *req)
{
    cJSON *json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "status", "ok");
    char *resp_str = cJSON_Print(json);
    ESP_ERROR_CHECK(httpd_resp_set_type(req, "application/json"));
    ESP_ERROR_CHECK(httpd_resp_send(req, resp_str, strlen(resp_str)));

    cJSON_Delete(json);
    free(resp_str);

    return ESP_OK;
}

/**
 * Parse the request data and populate the request_data struct
 * return ESP_OK if successful, ESP_FAIL otherwise
 */
esp_err_t parse_request_data(const char *buf, request_data *data, cJSON *parsingError)
{
    auto jsonData = cJSON_Parse(buf);
    if (jsonData == NULL)
    {
        ESP_LOGE(Server::TAG, "Error parsing JSON");
        return ESP_FAIL;
    }

    cJSON *effect = cJSON_GetObjectItem(jsonData, "effect");
    cJSON *effectSpeed = cJSON_GetObjectItem(jsonData, "effectSpeed");
    cJSON *targetColor = cJSON_GetObjectItem(jsonData, "color");
    cJSON *targetBrightness = cJSON_GetObjectItem(jsonData, "brightness");

    // Check if the effect is present and is a number
    if (effect != NULL) {
        if (!cJSON_IsNumber(effect))
        {
            cJSON_AddStringToObject(parsingError, "effect", "Invalid effect format. Must be an integer");
            return ESP_FAIL;
        }
        data->effect = (Effect)effect->valueint;
    }

    // Check if the effectSpeed is present and is a number
    if (effectSpeed != NULL) {
        if (!cJSON_IsNumber(effectSpeed))
        {
            cJSON_AddStringToObject(parsingError, "effectSpeed", "Invalid effect speed format. Must be an integer");
            return ESP_FAIL;
        }
        data->effectSpeed = effectSpeed->valueint;
    }

    // Check if the targetBrightness is present and is a number
    if (targetBrightness != NULL){
        if (!cJSON_IsNumber(targetBrightness))
        {
            cJSON_AddStringToObject(parsingError, "targetBrightness", "Invalid brightness format. Must be an integer");
            return ESP_FAIL;
        }
        data->brightness = targetBrightness->valueint;
    }

    // Check if the targetColor is present and is an array of 3 integers
    if (targetColor != NULL)
    {
        if (!cJSON_IsArray(targetColor) || cJSON_GetArraySize(targetColor) != 3)
        {
            ESP_LOGE(Server::TAG, "Invalid color format");
            cJSON_AddStringToObject(parsingError, "targetColor", "Invalid color format. Must be an array of 3 integers");
            return ESP_FAIL;
        }

        int r = cJSON_GetArrayItem(targetColor, 0)->valueint;
        int g = cJSON_GetArrayItem(targetColor, 1)->valueint;
        int b = cJSON_GetArrayItem(targetColor, 2)->valueint;
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255)
        {
            ESP_LOGE(Server::TAG, "Invalid color values");
            cJSON_AddStringToObject(parsingError, "targetColor", "Invalid color values. Must be between 0 and 255");
            return ESP_FAIL;
        }

        data->color = RgbColor(r, g, b);
    }

    cJSON_Delete(jsonData);

    return ESP_OK;
}

esp_err_t send_error_response(httpd_req_t *req, cJSON *error)
{
    char *resp_str = cJSON_Print(error);

    esp_err_t err;
    if((err = httpd_resp_set_status(req, "400 Bad Request")) != ESP_OK) { 
        cJSON_Delete(error);
        free(resp_str);   
        return err;
    }
    if((err = httpd_resp_set_type(req, "application/json")) != ESP_OK) { 
        cJSON_Delete(error);
        free(resp_str);   
        return err;
    }
    if((err = httpd_resp_send(req, resp_str, strlen(resp_str))) != ESP_OK) { 
        cJSON_Delete(error);
        free(resp_str);   
        return err;
    }

    cJSON_Delete(error);
    free(resp_str);
    return ESP_OK;
}

/* An HTTP POST handler */
esp_err_t Server::color_handler(httpd_req_t *req)
{
    char buf[200];
    int ret, remaining = req->content_len;

    while (remaining > 0)
    {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, buf,
                                  MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry receiving if timeout occurred */
                continue;
            }
            return ESP_FAIL;
        }

        remaining -= ret;
    }

    ESP_LOGI(Server::TAG, "Received body: %s", buf);
    auto requestError = cJSON_CreateObject();

    // check if header is application/json
    char header_buf[100];
    if (httpd_req_get_hdr_value_str(req, "Content-Type", header_buf, sizeof(header_buf)) == ESP_OK)
    {
        if (strcmp(header_buf, "application/json") != 0)
        {
            cJSON_AddStringToObject(requestError, "Content-Type", "Invalid content type. Must be application/json");
            return send_error_response(req, requestError);
        }
    }

    auto jsonData = cJSON_Parse(buf);
    if (jsonData == NULL)
    {
        cJSON_AddStringToObject(requestError, "body", "Invalid JSON format");
        auto result = send_error_response(req, requestError);
        cJSON_Delete(jsonData);
        return result;
    }

    request_data data = default_request_data();
    if(parse_request_data(buf, &data, requestError) != ESP_OK)
    {
        auto result = send_error_response(req, requestError);
        cJSON_Delete(jsonData);
        return result;
    }
    auto self = (Server *)req->user_ctx;
    if (data.effectSpeed.has_value())
    {
        self->controller.setEffectSpeed(data.effectSpeed.value());
    }
    if (data.color.has_value())
    {
        self->controller.setTargetColor(data.color.value());
    }
    if (data.brightness.has_value())
    {
        self->controller.setTargetBrightness(data.brightness.value());
    }    
    if (data.effect.has_value())
    {
        ESP_LOGI(Server::TAG, "Setting effect to %d", data.effect.value());
        self->controller.setEffect(data.effect.value());
    }

    ESP_ERROR_CHECK(httpd_resp_send(req, NULL, 0));
    cJSON_Delete(requestError);
    cJSON_Delete(jsonData);
    return ESP_OK;
}
