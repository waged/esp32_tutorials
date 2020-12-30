#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h" //"esp_event_loop.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "cJSON.h"
#include "sdkconfig.h"

#define DEVICE_INFO_SERVICE_UUID 0x180A

#define MANUFACTURER_NAME_CHAR 0xFEF4

#define CREDENTIALS_CHAR 0xDEAD

#define BLE_NAME "IoT-BLE"

char *TAG = "BLE-CONNECTION";

uint8_t ble_addr_type;

char *wifi_ssid;
char *wifi_psk;

void ble_app_advertise(void);

// callback from characteristic 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff
static int device_write(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    // data pattern {"SSID":"PUT_YOUR_WIFI_NAME", "PSK":"PUT_YOUR_WIFI_PASS"}
    char *incoming_data = (char *)ctxt->om->om_data;
    printf("incoming message: %s\n", incoming_data);
    cJSON *payload = cJSON_Parse(incoming_data);
    cJSON *ssid = cJSON_GetObjectItem(payload, "SSID");
    cJSON *psk = cJSON_GetObjectItem(payload, "PSK");
    wifi_ssid = ssid->valuestring;
    wifi_psk = psk->valuestring;
    printf("WiFi Credentials SSID:(%s) & PSK: (%s)\n", wifi_ssid, wifi_psk); 
    return 0;
}
static int device_info(uint16_t con_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    os_mbuf_append(ctxt->om, "your comapny name", strlen("your comapny name"));
    return 0;
}
static const struct ble_gatt_svc_def gatt_svcs[] = {
    {.type = BLE_GATT_SVC_TYPE_PRIMARY,
     .uuid = BLE_UUID16_DECLARE(DEVICE_INFO_SERVICE_UUID),
     .characteristics = (struct ble_gatt_chr_def[]){
         {.uuid = BLE_UUID16_DECLARE(MANUFACTURER_NAME_CHAR),
          .flags = BLE_GATT_CHR_F_READ,
          .access_cb = device_info},
         {.uuid = BLE_UUID16_DECLARE(CREDENTIALS_CHAR),
          .flags = BLE_GATT_CHR_F_WRITE,
          .access_cb = device_write},
         {0}}},
    {0}};
static int ble_gap_event(struct ble_gap_event *event, void *arg)
{
    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT CONNECT %s", event->connect.status == 0 ? "OK!" : "FAILED!");
        if (event->connect.status != 0)
        {
            //start advertising again!
            ble_app_advertise();
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        ble_app_advertise();
        break;
    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI("GAP", "BLE GAP EVENT");
        break;
    default:
        break;
    }
    return 0;
}

void ble_app_advertise(void)
{
    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof(fields));
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_DISC_LTD;
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;
    fields.name = (uint8_t *)ble_svc_gap_device_name();
    fields.name_len = strlen(ble_svc_gap_device_name());
    fields.name_is_complete = 1;
    ble_gap_adv_set_fields(&fields);
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    ble_gap_adv_start(ble_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event, NULL);
}

void ble_app_on_sync(void)
{
    // ble_addr_t addr;
    // ble_hs_id_gen_rnd(1, &addr);
    // ble_hs_id_set_rnd(addr.val);
    ble_hs_id_infer_auto(0, &ble_addr_type); // determines automatic address.
    ble_app_advertise();                     //start advertising the services -->
}

void host_task(void *param)
{
    nimble_port_run();
}

//BLE
void app_main()
{
    nvs_flash_init();
    esp_nimble_hci_and_controller_init();                   //initialize bluetooth controller.
    nimble_port_init();                                     //nimble library initialization.
    ESP_ERROR_CHECK(ble_svc_gap_device_name_set(BLE_NAME)); //set BLE name.
    ble_svc_gap_init();                                     //initialize the gap service.
    ble_svc_gatt_init();                                    //initailize the gatt service.
    ble_gatts_count_cfg(gatt_svcs);                         // config all the gatt services that wanted to be used.
    ble_gatts_add_svcs(gatt_svcs);                          // queues all services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;
    nimble_port_freertos_init(host_task);
}