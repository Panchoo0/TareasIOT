#include <stdio.h>
#include <string.h>


#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "nvs_flash.h"
#include "lwip/sockets.h" // Para sockets

#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "esp_mac.h"


//Credenciales de WiFi

#define WIFI_SSID "iotcositas"
#define WIFI_PASSWORD "iotcositas"
#define SERVER_IP     "192.168.4.1" // IP del servidor
#define SERVER_PORT   1234
#define SERVER_UDP_PORT 1235

// Variables de WiFi
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
static const char* TAG = "WIFI";
static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;


void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Intentando conectar a la red...");
    } else if (event_base == WIFI_EVENT &&
               event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 10) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(char* ssid, char* password) {
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
    strcpy((char*)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char*)wifi_config.sta.password, WIFI_PASSWORD);
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

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", ssid,
                 password);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid,
                 password);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}

void nvs_init() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

u_char gen_batt_lvl(){
    return (u_char) rand() % 100;
}

// void gen_time_stamp(char *time_stamp){
//     time_t t = time(NULL);
//     struct tm tm = *localtime(&t);
//     sprintf(time_stamp, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900,
//             tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
// }

int gen_tmp(){
    return (rand() % 25) + 5;
}
float gen_rand_float(float range, int min) {
    return ((float)rand()/(float)(RAND_MAX/range)) + min;
    
} 
int gen_hum(){
    return (rand() % 50) + 30;
}

int gen_pres(){
    return (rand() % 200) + 1000;
}

float gen_co(){
    return gen_rand_float(170, 30);
}

float gen_ampx(){
    return gen_rand_float(0.12 - 0.0059, 0.0059);
}

float gen_ampy(){
    return gen_rand_float(0.11 - 0.0041, 0.0041);
}

float gen_ampz(){
    return gen_rand_float(0.15 - 0.008, 0.008);
}

float gen_freqx(){
    return gen_rand_float(2, 29);
}

float gen_freqy(){
    return gen_rand_float(2, 59);
}

float gen_freqz(){
    return gen_rand_float(2, 89);
}

float rms(float x, float y, float z){
    return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
}

void acc(int *data) {
    for (int i =0; i < 2000; i++) {
        float num = gen_rand_float(32, -16);
        data[i] = *((int*) &num);
    }
}

void rgyro(int *data) {
    for (int i =0; i < 2000; i++) {
        float num = gen_rand_float(2000, -1000);
        data[i] = *((int*)&num);
    }
}

void mac(uint8_t *base_mac_addr) {
    esp_err_t ret = ESP_OK;
    esp_read_mac(base_mac_addr, ESP_MAC_WIFI_STA);
}

void set_headers(char *headers, char *ID_protocol, char *Transport_Layer) {
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

    if (strcmp(Transport_Layer, "UDP") == 0) {
        headers[8] = 0;
    } else {
        headers[8] = 1;
    }

    headers[9] = (char)atoi(ID_protocol);
    printf("ID Protocol: %d\n", headers[9]);
}

void set_protocol_0(char *message, char* ID_protocol, char* Transport_Layer){
    message[10] = 13; // Tamaño del mensaje
    message[11] = 0; // Tamano del mensaje
    message[12] = (char) gen_batt_lvl(); // batt lvl
    printf("Batt lvl: %d\n", message[12]);
    set_headers(message, ID_protocol, Transport_Layer);
}

void set_protocol_1(char *message, char* ID_protocol, char* Transport_Layer){
    set_headers(message, ID_protocol, Transport_Layer);
    set_protocol_0(message, ID_protocol, Transport_Layer);

    message[10] = 17; // Tamaño del mensaje
    message[11] = 0; // Tamano del mensaje

    // Agregar timestamp --> gettimeofday() <--
    message[13] = 0;
    message[14] = 0;
    message[15] = 0;
    message[16] = 0;
}

void set_protocol_2(char *message, char* ID_protocol, char* Transport_Layer){
    set_headers(message, ID_protocol, Transport_Layer);
    set_protocol_1(message, ID_protocol, Transport_Layer);

    message[10] = 27; // Tamaño del mensaje
    message[11] = 0; // Tamano del mensaje

    message[17] = (char) gen_tmp(); // temp
    printf("Temperatura: %d\n", message[17]);
    int press = gen_pres();
    printf("Presion: %i\n", press);

    message[18] = (char) (press >> 24 & 0xFF);
    message[19] = (char) (press >> 16 & 0xFF);
    message[20] = (char) (press >> 8 & 0xFF);
    message[21] = (char) (press & 0xFF);

    printf("p1: %d\n", message[18]);
    printf("p2: %d\n", message[19]);
    printf("p3: %d\n", message[20]);
    printf("p4: %d\n", message[21]);

    message[22] = (char) gen_hum(); // hum
    printf("Humedad: %d\n", message[22]);
    float fco = gen_co();
    int co = *((int*)&fco);
    message[23] = (char) (co >> 24 & 0xFF);
    message[24] = (char) (co >> 16 & 0xFF);
    message[25] = (char) (co >> 8 & 0xFF);
    message[26] = (char) (co & 0xFF);
    printf("CO: %f\n", fco);

}

void set_protocol_3(char *message,  char* ID_protocol, char* Transport_Layer){
    set_headers(message, ID_protocol, Transport_Layer);
    set_protocol_2(message, ID_protocol, Transport_Layer);

    message[10] = 55; // Tamaño del mensaje
    message[11] = 0; // Tamano del mensaje

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

    int ampx = *((int*)&fampx);
    int ampy = *((int*)&fampy);
    int ampz = *((int*)&fampz);

    int freqx = *((int*)&ffreqx);
    int freqy = *((int*)&ffreqy);
    int freqz = *((int*)&ffreqz);

    float frms = rms(fampx, fampy, fampz);
    int rms = *((int*)&frms);

    printf("RMS: %f\n", frms);

    message[27] = (char) (rms >> 24 & 0xFF);
    message[28] = (char) (rms >> 16 & 0xFF);
    message[29] = (char) (rms >> 8 & 0xFF);
    message[30] = (char) (rms & 0xFF);

    message[31] = (char) (ampx >> 24 & 0xFF);
    message[32] = (char) (ampx >> 16 & 0xFF);
    message[33] = (char) (ampx >> 8 & 0xFF);
    message[34] = (char) (ampx & 0xFF);

    message[35] = (char) (freqx >> 24 & 0xFF);
    message[36] = (char) (freqx >> 16 & 0xFF);
    message[37] = (char) (freqx >> 8 & 0xFF);
    message[38] = (char) (freqx & 0xFF);

    message[39] = (char) (ampy >> 24 & 0xFF);
    message[40] = (char) (ampy >> 16 & 0xFF);
    message[41] = (char) (ampy >> 8 & 0xFF);
    message[42] = (char) (ampy & 0xFF);

    message[43] = (char) (freqy >> 24 & 0xFF);
    message[44] = (char) (freqy >> 16 & 0xFF);
    message[45] = (char) (freqy >> 8 & 0xFF);
    message[46] = (char) (freqy & 0xFF);

    message[47] = (char) (ampz >> 24 & 0xFF);
    message[48] = (char) (ampz >> 16 & 0xFF);
    message[49] = (char) (ampz >> 8 & 0xFF);
    message[50] = (char) (ampz & 0xFF);

    message[51] = (char) (freqz >> 24 & 0xFF);
    message[52] = (char) (freqz >> 16 & 0xFF);
    message[53] = (char) (freqz >> 8 & 0xFF);
    message[54] = (char) (freqz & 0xFF);


}

void set_protocol_4(char *message, char* ID_protocol, char* Transport_Layer){
    set_headers(message, ID_protocol, Transport_Layer);
    set_protocol_2(message, ID_protocol, Transport_Layer);

    int size = 48027;
    message[10] = (char) (size >> 16 & 0xFF); // Tamaño del mensaje
    message[11] = (char) (size & 0xFF); // Tamano del mensaje

    int acc_x[2000];
    int acc_y[2000];
    int acc_z[2000];
    acc(acc_x);
    acc(acc_y);
    acc(acc_z);

    int gyro_x[2000];
    int gyro_y[2000];
    int gyro_z[2000];
    rgyro(gyro_x);
    rgyro(gyro_y);
    rgyro(gyro_z);

    for (int i = 0; i < 2000; i++) {
        message[27 + i] = (char) (acc_x[i] >> 24 & 0xFF);
        message[27 + (i + 1) * 4] = (char) (acc_x[i] >> 16 & 0xFF);
        message[27 + (i + 2) * 4] = (char) (acc_x[i] >> 8 & 0xFF);
        message[27 + (i + 3) * 4] = (char) (acc_x[i] & 0xFF);

        message[27 + 8000 + i] = (char) (acc_y[i] >> 24 & 0xFF);
        message[27 + 8000 + (i + 1) * 4] = (char) (acc_y[i] >> 16 & 0xFF);
        message[27 + 8000 + (i + 2) * 4] = (char) (acc_y[i] >> 8 & 0xFF);
        message[27 + 8000 + (i + 3) * 4] = (char) (acc_y[i] & 0xFF);

        message[27 + 16000 + i] = (char) (acc_z[i] >> 24 & 0xFF);
        message[27 + 16000 + (i + 1) * 4] = (char) (acc_z[i] >> 16 & 0xFF);
        message[27 + 16000 + (i + 2) * 4] = (char) (acc_z[i] >> 8 & 0xFF);
        message[27 + 16000 + (i + 3) * 4] = (char) (acc_z[i] & 0xFF);

        message[27 + 24000 + i] = (char) (gyro_x[i] >> 24 & 0xFF);
        message[27 + 24000 + (i + 1) * 4] = (char) (gyro_x[i] >> 16 & 0xFF);
        message[27 + 24000 + (i + 2) * 4] = (char) (gyro_x[i] >> 8 & 0xFF);
        message[27 + 24000 + (i + 3) * 4] = (char) (gyro_x[i] & 0xFF);

        message[27 + 32000 + i] = (char) (gyro_y[i] >> 24 & 0xFF);
        message[27 + 32000 + (i + 1) * 4] = (char) (gyro_y[i] >> 16 & 0xFF);
        message[27 + 32000 + (i + 2) * 4] = (char) (gyro_y[i] >> 8 & 0xFF);
        message[27 + 32000 + (i + 3) * 4] = (char) (gyro_y[i] & 0xFF);

    }
}

int get_procotol_length(char *ID_protocol){
    if (strcmp(ID_protocol, "0") == 0){
        return 13;
    } else if (strcmp(ID_protocol, "1") == 0){
        return 17;
    } else if (strcmp(ID_protocol, "2") == 0){
        return 27;
    } else if (strcmp(ID_protocol, "3") == 0){
        return 55;
    } else if (strcmp(ID_protocol, "4") == 0){
        return 48027;
    }
    return 0;
}

char *set_message(char* ID_protocol, char* Transport_Layer){
    char *message = NULL;
    if (strcmp(ID_protocol, "0") == 0){
        message = (char *) malloc(13 * sizeof(char));
        set_protocol_0(message, ID_protocol, Transport_Layer);
    } else if (strcmp(ID_protocol, "1") == 0){
        message = (char *) malloc(17 * sizeof(char));
        set_protocol_1(message, ID_protocol, Transport_Layer);
    } else if (strcmp(ID_protocol, "2") == 0){
        message = (char *) malloc(27 * sizeof(char));
        set_protocol_2(message, ID_protocol, Transport_Layer);
    } else if (strcmp(ID_protocol, "3") == 0){
        message = (char *) malloc(55 * sizeof(char));
        set_protocol_3(message, ID_protocol, Transport_Layer);
    } else if (strcmp(ID_protocol, "4") == 0){
        message = (char *) malloc(48027 * sizeof(char));
        set_protocol_4(message, ID_protocol, Transport_Layer);
    }

    return message;
}

void socket_udp(){
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_UDP_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

    // Crear un socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Error al crear el socket");
        return;
    }

    // Enviar mensaje continuamente hasta que se cambie el protocolo
    sendto(sock, "hola mundo", strlen("hola mundo"), 0,
           (struct sockaddr *)&server_addr, sizeof(server_addr));


    ESP_LOGI(TAG, "Se enviaron los datos");
    close(sock);
}


void socket_tcp(){
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr.s_addr);

    // Crear un socket
    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Error al crear el socket");
        return;
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        ESP_LOGE(TAG, "Error al conectar");
        close(sock);
        return;
    }


    ESP_LOGI(TAG, "Esperando a recibir los datos\n");
    char rx_buffer[128];
    int rx_len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    if (rx_len < 0) {
        ESP_LOGE(TAG, "Error al recibir datos\n");
        return;
    }
    ESP_LOGI(TAG, "Datos recibidos: %s\n", rx_buffer);

    char *tokens = strtok(rx_buffer, ":");

    char *ID_protocol = tokens;
    tokens = strtok(NULL, ":");
    char *Transport_Layer = tokens;

    if (strcmp(Transport_Layer, "UDP") == 0){
        close(sock);
        ESP_LOGI(TAG, "Protocolo UDP\n");
        return socket_udp();
    }

    ESP_LOGI(TAG, "Protocolo TCP\n");
    ESP_LOGI(TAG, "Protocolo %s\n", ID_protocol);
    char *message = set_message(ID_protocol, Transport_Layer);
    int size = get_procotol_length(ID_protocol);

    ESP_LOGI(TAG, "Largo %d\n", size);

    send(sock, message, size, 0);
    ESP_LOGI(TAG, "Se enviaron los datos\n");
    free(message);

    // esp_deep_sleep_enable_timer_wakeup(60000000); // 10000000 us = 10 s

    // esp_deep_sleep_start();
    esp_deep_sleep(60000000);


}



void app_main(void){
    nvs_init();
    wifi_init_sta(WIFI_SSID, WIFI_PASSWORD);
    ESP_LOGI(TAG,"Conectado a WiFi!\n");
    srand(time(NULL));
    socket_tcp();

    
}
