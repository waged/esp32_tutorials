#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event_loop.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"

char *TAG = "CONNECTION";

xSemaphoreHandle connectBinSemaphore;
bool isWifiConnectedWithIp = false;
bool isHttpCalledCalled = false;

esp_err_t clientEvent(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER");
        printf("%.*s", evt->data_len, (char *)evt->data);
        break;
    case HTTP_EVENT_ON_DATA:
        // ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        // if (!esp_http_client_is_chunked_response(evt->client))
        // {
        printf("%.*s", evt->data_len, (char *)evt->data);
        isHttpCalledCalled = true;
        // }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG, "connecting...\n");
        break;

    case SYSTEM_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "connected\n");
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip\n");
        isWifiConnectedWithIp = true;
        xSemaphoreGive(connectBinSemaphore); //this will call directly the OnConnected Function!
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "disconnected\n");
        break;

    default:
        break;
    }
    return ESP_OK;
}

void OnConnected(void *para)
{
    while (true)
    {
        if (xSemaphoreTake(connectBinSemaphore, 10000 / portTICK_RATE_MS))
        {
            ESP_LOGI(TAG, "connected");
            xSemaphoreTake(connectBinSemaphore, portMAX_DELAY); //release this thread so we can make sure that CPU never perform any further process.
        }
        else
        {
            ESP_LOGE(TAG, "Failed to connect. Retry in");
            for (int i = 0; i < 5; i++)
            {
                ESP_LOGE(TAG, "...%d", i);
                vTaskDelay(1000 / portTICK_RATE_MS);
            }
            esp_restart();
        }
    }
}

void wifiInit()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t wifi_config =
        {
            .sta = {
                .ssid = "Waged's iPhone",
                .password = "wegzwegz"}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    ESP_ERROR_CHECK(esp_wifi_start());
}
void app_main()
{
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    connectBinSemaphore = xSemaphoreCreateBinary();
    wifiInit();
    xTaskCreate(&OnConnected, "Handel WiFi", 1024 * 3, NULL, 5, NULL);
    while (1)
    {
        if (isWifiConnectedWithIp && !isHttpCalledCalled)
        {
            esp_http_client_config_t clientConfig = {
                .url = "https://google.com",
                .event_handler = clientEvent

            };
            esp_http_client_handle_t client = esp_http_client_init(&clientConfig);
            esp_http_client_perform(client);
            esp_http_client_cleanup(client);
        }
        if (isHttpCalledCalled)
        {
            printf("HTTP Request done !!! \n");
        }
        else
        {
            printf("Checking internet flag .... \n");
        }
        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}