
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h" // Para sockets

#include <sys/time.h>
#include <math.h>
#include "esp_mac.h"
#include "esp_sntp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"

#define GATTS_TABLE_TAG "GATTS_TABLE_DEMO"

#define WIFI_SSID "TeoyKala 2.4"
#define WIFI_PASSWORD "208470701G"
#define SERVER_IP "192.168.1.92" // IP del servidor
#define SERVER_PORT 1234
#define SERVER_UDP_PORT 1235

// Variables de WiFi
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
static const char *TAG = "WIFI";
static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;

#define PROFILE_NUM 1
#define PROFILE_APP_IDX 0
#define ESP_APP_ID 0x55
#define SAMPLE_DEVICE_NAME "ESP_GATTS_DEMO"
#define SVC_INST_ID 0

/* The max length of characteristic value. When the GATT client performs a write or prepare write operation,
 *  the data length must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
 */
#define GATTS_DEMO_CHAR_VAL_LEN_MAX 500
#define PREPARE_BUF_MAX_SIZE 1024
#define CHAR_DECLARATION_SIZE (sizeof(uint8_t))

#define ADV_CONFIG_FLAG (1 << 0)
#define SCAN_RSP_CONFIG_FLAG (1 << 1)

static uint8_t adv_config_done = 0;

void socket_tcp();

enum
{
    IDX_SVC,
    // IDX_CHAR_A,
    // IDX_CHAR_VAL_A,
    // IDX_CHAR_CFG_A,

    IDX_CHAR_STATUS,
    IDX_CHAR_VAL_STATUS,

    IDX_CHAR_ID_PROTOCOL,
    IDX_CHAR_VAL_ID_PROTOCOL,

    IDX_CHAR_BMI270_SAMPLING,
    IDX_CHAR_VAL_BMI270_SAMPLING,

    IDX_CHAR_BMI270_ACC,
    IDX_CHAR_VAL_BMI270_ACC,

    IDX_CHAR_BMI270_GYR,
    IDX_CHAR_VAL_BMI270_GYR,

    IDX_CHAR_BMI688_SAMPLING,
    IDX_CHAR_VAL_BMI688_SAMPLING,

    IDX_CHAR_DISCONTINUOUS_TIME,
    IDX_CHAR_VAL_DISCONTINUOUS_TIME,

    IDX_CHAR_PORT_TCP,
    IDX_CHAR_VAL_PORT_TCP,

    IDX_CHAR_PORT_UDP,
    IDX_CHAR_VAL_PORT_UDP,

    IDX_CHAR_HOST_IP,
    IDX_CHAR_VAL_HOST_IP,

    IDX_CHAR_SSID,
    IDX_CHAR_VAL_SSID,

    IDX_CHAR_PASS,
    IDX_CHAR_VAL_PASS,

    HRS_IDX_NB,
};

uint16_t heart_rate_handle_table[HRS_IDX_NB];

typedef struct
{
    uint8_t *prepare_buf;
    int prepare_len;
} prepare_type_env_t;

static prepare_type_env_t prepare_write_env;

#define CONFIG_SET_RAW_ADV_DATA
#ifdef CONFIG_SET_RAW_ADV_DATA
static uint8_t raw_adv_data[] = {
    /* flags */
    0x02, 0x01, 0x06,
    /* tx power*/
    0x02, 0x0a, 0xeb,
    /* service uuid */
    0x03, 0x03, 0xFF, 0x00,
    /* device name */
    0x0f, 0x09, 'E', 'S', 'P', '_', 'G', 'A', 'T', 'T', 'S', '_', 'D', 'E', 'M', 'O'};
static uint8_t raw_scan_rsp_data[] = {
    /* flags */
    0x02, 0x01, 0x06,
    /* tx power */
    0x02, 0x0a, 0xeb,
    /* service uuid */
    0x03, 0x03, 0xFF, 0x00};

#else
static uint8_t service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xFF,
    0x00,
    0x00,
    0x00,
};

/* The length of adv data must be less than 31 bytes */
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, // slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, // test_manufacturer,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(service_uuid),
    .p_service_uuid = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(service_uuid),
    .p_service_uuid = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
#endif /* CONFIG_SET_RAW_ADV_DATA */

static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst
{
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

int Write_NVS(int32_t data, int key)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES)
    {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    // Open
    // printf("Opening NVS .. ");
    nvs_handle_t my_handle;
    err = nvs_open("Storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%d) opening NVS handle!\n", err);
        return -1;
    }
    else
    {
        // printf("Done\n");
        // // Write
        // printf("Updating restart counter in NVS ... ");
        switch (key)
        {
        case 1:
            printf("status\n");
            err = nvs_set_i32(my_handle, "status", data);
            break;
        case 2:
            err = nvs_set_i32(my_handle, "Samp_Freq", data);
            break;
        case 3:
            err = nvs_set_i32(my_handle, "T_s", data);
            break;
        case 4:
            err = nvs_set_i32(my_handle, "Acc_Sen", data);
            break;
        case 5:
            err = nvs_set_i32(my_handle, "Gyro_Sen", data);
            break;
        case 6:
            err = nvs_set_i32(my_handle, "Acc_Any", data);
            break;
        case 7:
            err = nvs_set_i32(my_handle, "Rf_Cal", data);
            break;
        case 8:
            err = nvs_set_i32(my_handle, "SEL_ID", data);
            break;

        default:
            printf("ERROR key");
            break;
        }
        printf((err != ESP_OK) ? "Failed in NVS!\n" : "Done\n");
        // printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        // Close
        nvs_close(my_handle);
    }
    fflush(stdout);
    return 0;
}

int Read_NVS(int32_t *data, int key)
{
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    ESP_ERROR_CHECK(err);

    // Open
    // printf("\n");
    // printf("Opening Non-Volatile Storage (NVS) handle... ");
    nvs_handle_t my_handle;
    err = nvs_open("Storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        printf("Error (%d) opening NVS handle!\n", err);
        return -1;
    }
    else
    {
        // printf("Done\n");

        // // Read
        // printf("Reading from NVS ... ");
        switch (key)
        {
        case 1:
            err = nvs_get_i32(my_handle, "status", data);
            break;
        case 2:
            err = nvs_get_i32(my_handle, "Samp_Freq", data);
            break;
        case 3:
            err = nvs_get_i32(my_handle, "T_s", data);
            break;
        case 4:
            err = nvs_get_i32(my_handle, "Acc_Sen", data);
            break;
        case 5:
            err = nvs_get_i32(my_handle, "Gyro_Sen", data);
            break;
        case 6:
            err = nvs_get_i32(my_handle, "Acc_Any", data);
            break;
        case 7:
            err = nvs_get_i32(my_handle, "Rf_Cal", data);
            break;
        case 8:
            err = nvs_get_i32(my_handle, "SEL_ID", data);
            break;

        default:
            printf("ERROR key");
            break;
        }
        switch (err)
        {
        case ESP_OK:
            // printf("Done\n");
            //  printf("Value Data = %d\n", *data);
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            printf("The value is not initialized yet!\n");
            break;
        default:
            printf("Error (%d) reading!\n", err);
        }
        // printf("Committing updates in NVS ... ");
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        // Close
        nvs_close(my_handle);
    }
    fflush(stdout);
    return 0;
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
                                        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst heart_rate_profile_tab[PROFILE_NUM] = {
    [PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE, /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

/* Service */
static const uint16_t GATTS_SERVICE_UUID_TEST = 0x00FF;
static const uint16_t GATTS_CHAR_UUID_STATUS = 0xFF01;
static const uint16_t GATTS_CHAR_UUID_ID_PROTOCOL = 0xFF02;
static const uint16_t GATTS_CHAR_UUID_BMI270_SAMPLING = 0xFF03;
static const uint16_t GATTS_CHAR_UUID_BMI270_ACC = 0xFF04;
static const uint16_t GATTS_CHAR_UUID_BMI270_GYR = 0xFF05;
static const uint16_t GATTS_CHAR_UUID_BMI688_SAMPLING = 0xFF06;
static const uint16_t GATTS_CHAR_UUID_DISCONTINUOUS_TIME = 0xFF07;
static const uint16_t GATTS_CHAR_UUID_PORT_TCP = 0xFF08;
static const uint16_t GATTS_CHAR_UUID_PORT_UDP = 0xFF09;
static const uint16_t GATTS_CHAR_UUID_HOST_IP = 0xFF0A;
static const uint16_t GATTS_CHAR_UUID_SSID = 0xFF0B;
static const uint16_t GATTS_CHAR_UUID_PASS = 0xFF0C;

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint8_t char_prop_read_write_notify = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t STATUS[1] = {0};
static const uint8_t ID_PROTOCOL[1] = {1};
static const uint32_t BMI270_SAMPLING[1] = {0};
static const uint32_t BMI270_ACC[1] = {0};
static const uint32_t BMI270_GYR[1] = {0};
static const uint32_t BMI688_SAMPLING[1] = {0};
static const uint32_t DISCONTINUOUS_TIME[1] = {0};
static const uint32_t PORT_TCP[1] = {0};
static const uint32_t PORT_UDP[1] = {0};
static const uint32_t HOST_IP[1] = {0};
static const uint32_t SSID[1] = {0};
static const uint32_t PASS[1] = {0};

/* Full Database Description - Used to add attributes into the database */
static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] =
    {
        // Service Declaration
        [IDX_SVC] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(GATTS_SERVICE_UUID_TEST), (uint8_t *)&GATTS_SERVICE_UUID_TEST}},

        /* Characteristic Declaration */
        [IDX_CHAR_STATUS] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

        /* Characteristic Value */
        [IDX_CHAR_VAL_STATUS] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_STATUS, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(STATUS), (uint8_t *)STATUS}},

        /* Characteristic Declaration */
        [IDX_CHAR_ID_PROTOCOL] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},
        /* Characteristic Value */
        [IDX_CHAR_VAL_ID_PROTOCOL] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_ID_PROTOCOL, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(ID_PROTOCOL), (uint8_t *)ID_PROTOCOL}},

        /* Characteristic Declaration */
        [IDX_CHAR_BMI270_SAMPLING] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},
        /* Characteristic Value */
        [IDX_CHAR_VAL_BMI270_SAMPLING] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_BMI270_SAMPLING, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(BMI270_SAMPLING), (uint32_t *)BMI270_SAMPLING}},

        /* Characteristic Declaration */
        [IDX_CHAR_BMI270_ACC] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

        /* Characteristic Value */
        [IDX_CHAR_VAL_BMI270_ACC] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_BMI270_ACC, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(BMI270_ACC), (uint32_t *)BMI270_ACC}},

        /* Characteristic Declaration */
        [IDX_CHAR_BMI270_GYR] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

        /* Characteristic Value */
        [IDX_CHAR_VAL_BMI270_GYR] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_BMI270_GYR, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(BMI270_GYR), (uint32_t *)BMI270_GYR}},

        /* Characteristic Declaration */
        [IDX_CHAR_BMI688_SAMPLING] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

        /* Characteristic Value */
        [IDX_CHAR_VAL_BMI688_SAMPLING] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_BMI688_SAMPLING, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(BMI688_SAMPLING), (uint32_t *)BMI688_SAMPLING}},

        /* Characteristic Declaration */
        [IDX_CHAR_DISCONTINUOUS_TIME] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

        /* Characteristic Value */
        [IDX_CHAR_VAL_DISCONTINUOUS_TIME] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_DISCONTINUOUS_TIME, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(DISCONTINUOUS_TIME), (uint32_t *)DISCONTINUOUS_TIME}},

        /* Characteristic Declaration */
        [IDX_CHAR_PORT_TCP] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},
        /* Characteristic Value */
        [IDX_CHAR_VAL_PORT_TCP] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_PORT_TCP, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(PORT_TCP), (uint32_t *)PORT_TCP}},

        /* Characteristic Declaration */
        [IDX_CHAR_PORT_UDP] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

        /* Characteristic Value */
        [IDX_CHAR_VAL_PORT_UDP] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_PORT_UDP, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(PORT_UDP), (uint32_t *)PORT_UDP}},

        /* Characteristic Declaration */
        [IDX_CHAR_HOST_IP] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

        /* Characteristic Value */
        [IDX_CHAR_VAL_HOST_IP] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_HOST_IP, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(HOST_IP), (uint32_t *)HOST_IP}},

        /* Characteristic Declaration */
        [IDX_CHAR_SSID] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

        /* Characteristic Value */
        [IDX_CHAR_VAL_SSID] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_SSID, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(SSID), (uint32_t *)SSID}},

        /* Characteristic Declaration */
        [IDX_CHAR_PASS] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},
        /* Characteristic Value */
        [IDX_CHAR_VAL_PASS] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_PASS, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(PASS), (uint32_t *)PASS}},

};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
#ifdef CONFIG_SET_RAW_ADV_DATA
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~ADV_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
#else
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~ADV_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
#endif
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        /* advertising start complete event to indicate advertising start successfully or failed */
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "advertising start failed");
        }
        else
        {
            ESP_LOGI(GATTS_TABLE_TAG, "advertising start successfully");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "Advertising stop failed");
        }
        else
        {
            ESP_LOGI(GATTS_TABLE_TAG, "Stop adv successfully");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(GATTS_TABLE_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                 param->update_conn_params.status,
                 param->update_conn_params.min_int,
                 param->update_conn_params.max_int,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC && prepare_write_env->prepare_buf)
    {
        ESP_LOG_BUFFER_HEX(GATTS_TABLE_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    }
    else
    {
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATT_PREP_WRITE_CANCEL");
    }
    if (prepare_write_env->prepare_buf)
    {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

void event_handler(void *arg, esp_event_base_t event_base,
                   int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Intentando conectar a la red...");
    }
    else if (event_base == WIFI_EVENT &&
             event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < 10)
        {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(char *ssid, char *password)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));

    // Set the specific fields
    strcpy((char *)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, WIFI_PASSWORD);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE, portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", ssid,
                 password);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid,
                 password);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT:
    {
        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(SAMPLE_DEVICE_NAME);
        if (set_dev_name_ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "set device name failed, error code = %x", set_dev_name_ret);
        }
#ifdef CONFIG_SET_RAW_ADV_DATA
        esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
        if (raw_adv_ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
        }
        adv_config_done |= ADV_CONFIG_FLAG;
        esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
        if (raw_scan_ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
        }
        adv_config_done |= SCAN_RSP_CONFIG_FLAG;
#else
        // config adv data
        esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
        }
        adv_config_done |= ADV_CONFIG_FLAG;
        // config scan response data
        ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
        if (ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "config scan response data failed, error code = %x", ret);
        }
        adv_config_done |= SCAN_RSP_CONFIG_FLAG;
#endif
        esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HRS_IDX_NB, SVC_INST_ID);
        if (create_attr_ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
        }
    }
    break;
    case ESP_GATTS_READ_EVT:
        break;
    case ESP_GATTS_WRITE_EVT:
        if (!param->write.is_prep)
        {
            // the data length of gattc write  must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
            ESP_LOGI(GATTS_TABLE_TAG, "GATT_WRITE_EVT, handle = %d, value len = %d, value :", param->write.handle, param->write.len);
            ESP_LOG_BUFFER_HEX(GATTS_TABLE_TAG, param->write.value, param->write.len);
            ESP_LOGI(GATTS_TABLE_TAG, "EXPECTED IDX: %d", heart_rate_handle_table[IDX_CHAR_PASS]);
            ESP_LOGI(GATTS_TABLE_TAG, "EXPECTED IDX_CHAR_VAL_PASS: %d", heart_rate_handle_table[IDX_CHAR_VAL_PASS]);

            if (heart_rate_handle_table[IDX_CHAR_VAL_PASS] == param->write.handle && param->write.len == 1)
            {
                int32_t data = {1};
                Write_NVS(data, 1);
            }
        }
        break;
    case ESP_GATTS_EXEC_WRITE_EVT:
        // the length of gattc prepare write data must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
        example_exec_write_event_env(&prepare_write_env, param);
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        break;
    case ESP_GATTS_CONF_EVT:
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONF_EVT, status = %d, attr_handle %d", param->conf.status, param->conf.handle);
        break;
    case ESP_GATTS_START_EVT:
        ESP_LOGI(GATTS_TABLE_TAG, "SERVICE_START_EVT, status %d, service_handle %d", param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_CONNECT_EVT:
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d", param->connect.conn_id);
        ESP_LOG_BUFFER_HEX(GATTS_TABLE_TAG, param->connect.remote_bda, 6);
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the iOS system, please refer to Apple official documents about the BLE connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.max_int = 0x20; // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10; // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;  // timeout = 400*10ms = 4000ms
        // start sent the update connection parameters to the peer device.
        esp_ble_gap_update_conn_params(&conn_params);
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GATTS_CREAT_ATTR_TAB_EVT:
    {
        if (param->add_attr_tab.status != ESP_GATT_OK)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "create attribute table failed, error code=0x%x", param->add_attr_tab.status);
        }
        else if (param->add_attr_tab.num_handle != HRS_IDX_NB)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "create attribute table abnormally, num_handle (%d) \
                        doesn't equal to HRS_IDX_NB(%d)",
                     param->add_attr_tab.num_handle, HRS_IDX_NB);
        }
        else
        {
            ESP_LOGI(GATTS_TABLE_TAG, "create attribute table successfully, the number handle = %d", param->add_attr_tab.num_handle);
            memcpy(heart_rate_handle_table, param->add_attr_tab.handles, sizeof(heart_rate_handle_table));
            esp_ble_gatts_start_service(heart_rate_handle_table[IDX_SVC]);
        }
        break;
    }
    case ESP_GATTS_STOP_EVT:
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    case ESP_GATTS_UNREG_EVT:
    case ESP_GATTS_DELETE_EVT:
    default:
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{

    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            heart_rate_profile_tab[PROFILE_APP_IDX].gatts_if = gatts_if;
        }
        else
        {
            ESP_LOGE(GATTS_TABLE_TAG, "reg app failed, app_id %04x, status %d",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }
    do
    {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++)
        {
            /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == heart_rate_profile_tab[idx].gatts_if)
            {
                if (heart_rate_profile_tab[idx].gatts_cb)
                {
                    heart_rate_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

u_char gen_batt_lvl()
{
    return (u_char)rand() % 100;
}

int gen_tmp()
{
    return (rand() % 25) + 5;
}

float gen_rand_float(float range, int min)
{
    return ((float)rand() / (float)(RAND_MAX / range)) + min;
}
int gen_hum()
{
    return (rand() % 50) + 30;
}

int gen_pres()
{
    return (rand() % 200) + 1000;
}

float gen_co()
{
    return gen_rand_float(170, 30);
}

float gen_ampx()
{
    return gen_rand_float(0.12 - 0.0059, 0.0059);
}

float gen_ampy()
{
    return gen_rand_float(0.11 - 0.0041, 0.0041);
}

float gen_ampz()
{
    return gen_rand_float(0.15 - 0.008, 0.008);
}

float gen_freqx()
{
    return gen_rand_float(2, 29);
}

float gen_freqy()
{
    return gen_rand_float(2, 59);
}

float gen_freqz()
{
    return gen_rand_float(2, 89);
}

float rms(float x, float y, float z)
{
    return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

void acc(int *data)
{
    for (int i = 0; i < 2000; i++)
    {
        float num = gen_rand_float(32, -16);
        data[i] = *((int *)&num);
    }
}

void rgyro(int *data)
{
    for (int i = 0; i < 2000; i++)
    {
        float num = gen_rand_float(2000, -1000);
        data[i] = *((int *)&num);
    }
}

void mac(uint8_t *base_mac_addr)
{
    esp_read_mac(base_mac_addr, ESP_MAC_WIFI_STA);
}

void set_headers(char *headers, char *ID_protocol, char *Transport_Layer)
{
    short id = 0;
    uint8_t base_mac_addr[6] = {0};
    mac(base_mac_addr);
    headers[0] = (char)id >> 8;
    headers[1] = (char)id & 0xFF;
    headers[2] = base_mac_addr[0];
    headers[3] = base_mac_addr[1];
    headers[4] = base_mac_addr[2];
    headers[5] = base_mac_addr[3];
    headers[6] = base_mac_addr[4];
    headers[7] = base_mac_addr[5];
    printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", base_mac_addr[0], base_mac_addr[1], base_mac_addr[2], base_mac_addr[3], base_mac_addr[4], base_mac_addr[5]);

    if (strcmp(Transport_Layer, "UDP") == 0)
    {
        headers[8] = 0;
    }
    else
    {
        headers[8] = 1;
    }

    headers[9] = (char)atoi(ID_protocol);
    printf("ID Protocol: %d\n", headers[9]);
}

void set_protocol_0(char *message, char *ID_protocol, char *Transport_Layer)
{
    message[10] = 0;                    // Tamaño del mensaje
    message[11] = 13;                   // Tamano del mensaje
    message[12] = (char)gen_batt_lvl(); // batt lvl
    printf("Batt lvl: %d\n", message[12]);
    set_headers(message, ID_protocol, Transport_Layer);
}

void set_protocol_1(char *message, char *ID_protocol, char *Transport_Layer)
{
    set_headers(message, ID_protocol, Transport_Layer);
    set_protocol_0(message, ID_protocol, Transport_Layer);

    message[10] = 0;  // Tamaño del mensaje
    message[11] = 17; // Tamano del mensaje

    // Agregar timestamp --> gettimeofday() <--
    message[13] = 0;
    message[14] = 0;
    message[15] = 0;
    message[16] = 0;
}

void set_protocol_2(char *message, char *ID_protocol, char *Transport_Layer)
{
    set_headers(message, ID_protocol, Transport_Layer);
    set_protocol_1(message, ID_protocol, Transport_Layer);

    message[10] = 0;  // Tamaño del mensaje
    message[11] = 27; // Tamano del mensaje

    message[17] = (char)gen_tmp(); // temp
    printf("Temperatura: %d\n", message[17]);
    int press = gen_pres();
    printf("Presion: %i\n", press);

    message[18] = (char)(press >> 24 & 0xFF);
    message[19] = (char)(press >> 16 & 0xFF);
    message[20] = (char)(press >> 8 & 0xFF);
    message[21] = (char)(press & 0xFF);

    message[22] = (char)gen_hum(); // hum
    printf("Humedad: %d\n", message[22]);
    float fco = gen_co();
    int co = *((int *)&fco);
    message[23] = (char)(co >> 24 & 0xFF);
    message[24] = (char)(co >> 16 & 0xFF);
    message[25] = (char)(co >> 8 & 0xFF);
    message[26] = (char)(co & 0xFF);
    printf("CO: %f\n", fco);
}

void set_protocol_3(char *message, char *ID_protocol, char *Transport_Layer)
{
    set_headers(message, ID_protocol, Transport_Layer);
    set_protocol_2(message, ID_protocol, Transport_Layer);

    message[10] = 0;  // Tamaño del mensaje
    message[11] = 55; // Tamano del mensaje

    float fampx = gen_ampx();
    float fampy = gen_ampy();
    float fampz = gen_ampz();

    printf("fampx: %f\n", fampx);
    printf("fampy: %f\n", fampy);
    printf("fampz: %f\n", fampz);

    float ffreqx = gen_freqx();
    float ffreqy = gen_freqy();
    float ffreqz = gen_freqz();

    printf("ffreqx: %f\n", ffreqx);
    printf("ffreqy: %f\n", ffreqy);
    printf("ffreqz: %f\n", ffreqz);

    int ampx = *((int *)&fampx);
    int ampy = *((int *)&fampy);
    int ampz = *((int *)&fampz);

    int freqx = *((int *)&ffreqx);
    int freqy = *((int *)&ffreqy);
    int freqz = *((int *)&ffreqz);

    float frms = rms(fampx, fampy, fampz);
    int rms = *((int *)&frms);

    printf("fampx as int: %d\n", ampx);

    printf("RMS: %f\n", frms);

    message[27] = (char)(rms >> 24 & 0xFF);
    message[28] = (char)(rms >> 16 & 0xFF);
    message[29] = (char)(rms >> 8 & 0xFF);
    message[30] = (char)(rms & 0xFF);

    message[31] = (char)(ampx >> 24 & 0xFF);
    message[32] = (char)(ampx >> 16 & 0xFF);
    message[33] = (char)(ampx >> 8 & 0xFF);
    message[34] = (char)(ampx & 0xFF);

    message[35] = (char)(freqx >> 24 & 0xFF);
    message[36] = (char)(freqx >> 16 & 0xFF);
    message[37] = (char)(freqx >> 8 & 0xFF);
    message[38] = (char)(freqx & 0xFF);

    message[39] = (char)(ampy >> 24 & 0xFF);
    message[40] = (char)(ampy >> 16 & 0xFF);
    message[41] = (char)(ampy >> 8 & 0xFF);
    message[42] = (char)(ampy & 0xFF);

    message[43] = (char)(freqy >> 24 & 0xFF);
    message[44] = (char)(freqy >> 16 & 0xFF);
    message[45] = (char)(freqy >> 8 & 0xFF);
    message[46] = (char)(freqy & 0xFF);

    message[47] = (char)(ampz >> 24 & 0xFF);
    message[48] = (char)(ampz >> 16 & 0xFF);
    message[49] = (char)(ampz >> 8 & 0xFF);
    message[50] = (char)(ampz & 0xFF);

    message[51] = (char)(freqz >> 24 & 0xFF);
    message[52] = (char)(freqz >> 16 & 0xFF);
    message[53] = (char)(freqz >> 8 & 0xFF);
    message[54] = (char)(freqz & 0xFF);
}

void print_first_20(int *data)
{
    for (int i = 0; i < 20; i++)
    {
        float num = *((float *)&data[i]);
        printf("%f ", num);
    }
    printf("\n");
}

void set_protocol_4(char *message, char *ID_protocol, char *Transport_Layer)
{
    set_headers(message, ID_protocol, Transport_Layer);
    set_protocol_2(message, ID_protocol, Transport_Layer);

    int size = 48027;
    message[10] = (char)(size >> 8 & 0xFF);
    message[11] = (char)(size & 0xFF);

    int *acc_x = malloc(2000 * sizeof(int));
    int *acc_y = malloc(2000 * sizeof(int));
    int *acc_z = malloc(2000 * sizeof(int));

    acc(acc_x);
    acc(acc_y);
    acc(acc_z);

    int *gyro_x = malloc(2000 * sizeof(int));
    int *gyro_y = malloc(2000 * sizeof(int));
    int *gyro_z = malloc(2000 * sizeof(int));

    rgyro(gyro_x);
    rgyro(gyro_y);
    rgyro(gyro_z);

    for (int i = 0; i < 2000; i++)
    {
        message[27 + i * 4] = (char)(acc_x[i] >> 24 & 0xFF);
        message[27 + i * 4 + 1] = (char)(acc_x[i] >> 16 & 0xFF);
        message[27 + i * 4 + 2] = (char)(acc_x[i] >> 8 & 0xFF);
        message[27 + i * 4 + 3] = (char)(acc_x[i] & 0xFF);

        message[27 + 8000 + i * 4] = (char)(acc_y[i] >> 24 & 0xFF);
        message[27 + 8000 + i * 4 + 1] = (char)(acc_y[i] >> 16 & 0xFF);
        message[27 + 8000 + i * 4 + 2] = (char)(acc_y[i] >> 8 & 0xFF);
        message[27 + 8000 + i * 4 + 3] = (char)(acc_y[i] & 0xFF);

        message[27 + 16000 + i * 4] = (char)(acc_z[i] >> 24 & 0xFF);
        message[27 + 16000 + i * 4 + 1] = (char)(acc_z[i] >> 16 & 0xFF);
        message[27 + 16000 + i * 4 + 2] = (char)(acc_z[i] >> 8 & 0xFF);
        message[27 + 16000 + i * 4 + 3] = (char)(acc_z[i] & 0xFF);

        message[27 + 24000 + i * 4] = (char)(gyro_x[i] >> 24 & 0xFF);
        message[27 + 24000 + i * 4 + 1] = (char)(gyro_x[i] >> 16 & 0xFF);
        message[27 + 24000 + i * 4 + 2] = (char)(gyro_x[i] >> 8 & 0xFF);
        message[27 + 24000 + i * 4 + 3] = (char)(gyro_x[i] & 0xFF);

        message[27 + 32000 + i * 4] = (char)(gyro_y[i] >> 24 & 0xFF);
        message[27 + 32000 + i * 4 + 1] = (char)(gyro_y[i] >> 16 & 0xFF);
        message[27 + 32000 + i * 4 + 2] = (char)(gyro_y[i] >> 8 & 0xFF);
        message[27 + 32000 + i * 4 + 3] = (char)(gyro_y[i] & 0xFF);
    }

    free(acc_x);
    free(acc_y);
    free(acc_z);
    free(gyro_x);
    free(gyro_y);
    free(gyro_z);
}

int get_procotol_length(char *ID_protocol)
{
    if (strcmp(ID_protocol, "0") == 0)
    {
        return 13;
    }
    else if (strcmp(ID_protocol, "1") == 0)
    {
        return 17;
    }
    else if (strcmp(ID_protocol, "2") == 0)
    {
        return 27;
    }
    else if (strcmp(ID_protocol, "3") == 0)
    {
        return 55;
    }
    else if (strcmp(ID_protocol, "4") == 0)
    {
        return 48027;
    }
    return 0;
}

char *set_message(char *ID_protocol, char *Transport_Layer, char *ID_message)
{
    char *message = NULL;
    if (strcmp(ID_protocol, "0") == 0)
    {
        message = (char *)malloc(13 * sizeof(char));
        set_protocol_0(message, ID_protocol, Transport_Layer);
    }
    else if (strcmp(ID_protocol, "1") == 0)
    {
        message = (char *)malloc(17 * sizeof(char));
        set_protocol_1(message, ID_protocol, Transport_Layer);
    }
    else if (strcmp(ID_protocol, "2") == 0)
    {
        message = (char *)malloc(27 * sizeof(char));
        set_protocol_2(message, ID_protocol, Transport_Layer);
    }
    else if (strcmp(ID_protocol, "3") == 0)
    {
        message = (char *)malloc(55 * sizeof(char));
        set_protocol_3(message, ID_protocol, Transport_Layer);
    }
    else if (strcmp(ID_protocol, "4") == 0)
    {
        printf("Protocolo 4\n");
        message = (char *)malloc(48027 * sizeof(char));
        printf("Creado arreglo \n");
        set_protocol_4(message, ID_protocol, Transport_Layer);
    }
    int id = atoi(ID_message);
    message[0] = (char)id >> 8;
    message[1] = (char)id & 0xFF;
    printf("ID message %d\n", id);

    return message;
}

int udp_conn(char *ID_protocol, char *Transport_Layer, char *ID_message)
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_UDP_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

    // Crear un socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Error al crear el socket");
        return 0;
    }

    char *message = set_message(ID_protocol, Transport_Layer, ID_message);
    int size = get_procotol_length(ID_protocol);

    ESP_LOGI(TAG, "Largo %d\n", size);

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

    sendto(sock, message, size, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    ESP_LOGI(TAG, "Se enviaron los datos");

    char rx_buffer[128];
    rx_buffer[0] = '\0';
    int rx_len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, NULL, NULL);
    rx_buffer[rx_len] = '\0';
    ESP_LOGI(TAG, "Datos recibidos (%d): %s\n", rx_len, rx_buffer);
    close(sock);
    if (rx_len < 0)
    {
    }
    else if (strcmp(rx_buffer, "TCP") == 0)
    {
        rx_buffer[rx_len] = '\0';
        ESP_LOGI(TAG, "Cambiando a Protocolo TCP\n");
        return 1;
    }

    return 0;
}

void socket_udp(char *ID_protocol, char *Transport_Layer, char *ID_message)
{
    while (udp_conn(ID_protocol, Transport_Layer, ID_message) == 0)
    {
    }
    esp_deep_sleep(10000000);
    return;
}

void socket_tcp()
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

    // Crear un socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Error al crear el socket");
        esp_deep_sleep(10000000);
        return;
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        ESP_LOGE(TAG, "Error al conectar");
        close(sock);
        esp_deep_sleep(10000000);
        return;
    }

    ESP_LOGI(TAG, "Esperando a recibir los datos\n");
    char rx_buffer[128];
    int rx_len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    if (rx_len < 0)
    {
        ESP_LOGE(TAG, "Error al recibir datos\n");
        esp_deep_sleep(10000000);
        return;
    }
    ESP_LOGI(TAG, "Datos recibidos: %s\n", rx_buffer);
    rx_buffer[rx_len] = 0;
    char *tokens = strtok(rx_buffer, ":");

    char *ID_protocol = tokens;
    tokens = strtok(NULL, ":");
    char *Transport_Layer = tokens;
    tokens = strtok(NULL, ":");
    char *ID_message = tokens;

    if (strcmp(Transport_Layer, "UDP") == 0)
    {
        close(sock);
        ESP_LOGI(TAG, "Protocolo UDP\n");
        return socket_udp(ID_protocol, Transport_Layer, ID_message);
    }

    ESP_LOGI(TAG, "Protocolo TCP\n");
    ESP_LOGI(TAG, "Protocolo %s\n", ID_protocol);

    char *message = set_message(ID_protocol, Transport_Layer, ID_message);
    int size = get_procotol_length(ID_protocol);

    ESP_LOGI(TAG, "Largo %d\n", size);

    if (size > 1000)
    {
        int sizeToSend = size;
        for (int i = 0; i < size; i += 1000)
        {
            if (sizeToSend < 1000)
            {
                int r = send(sock, message + i, sizeToSend, 0);
                if (r < 0)
                {
                    ESP_LOGE(TAG, "ERROR AL ENVIAR DATOS");
                    continue;
                }
                ESP_LOGI(TAG, "Se envió %d bytes\n", r);
                break;
            }
            int r = send(sock, message + i, 1000, 0);
            if (r < 0)
            {
                ESP_LOGE(TAG, "ERROR AL ENVIAR DATOS");
                continue;
            }
            sizeToSend -= 1000;
            ESP_LOGI(TAG, "Se envió %d bytes\n", r);
        }
    }
    else
    {
        int r = send(sock, message, size, 0);
    }

    ESP_LOGI(TAG, "Se enviaron los datos\n");
    free(message);

    close(sock);

    // esp_deep_sleep_enable_timer_wakeup(60000000); // 10000000 us = 10 s

    // esp_deep_sleep_start();
    esp_deep_sleep(10000000);
}

void app_main(void)
{
    esp_err_t ret;

    /* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    wifi_init_sta(WIFI_SSID, WIFI_PASSWORD);
    ESP_LOGI(TAG, "Conectado a WiFi!\n");
    srand(time(NULL));

    socket_tcp();

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "gatts register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "gap register error, error code = %x", ret);
        return;
    }

    ret = esp_ble_gatts_app_register(ESP_APP_ID);
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
}