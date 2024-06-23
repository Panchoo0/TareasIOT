#include "lwip/sockets.h" // Para sockets
#include "esp_sleep.h"
#include "gen_data.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_func.h"

#define WIFI_SSID "TeoyKala 2.4"
#define WIFI_PASSWORD "208470701G"
#define SERVER_IP "192.168.1.83" // IP del servidor
#define SERVER_PORT 1234
#define SERVER_UDP_PORT 1235

// Variables de WiFi
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
static const char *TAG = "WIFI";
static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;

int udp_conn(char ID_protocol, char status)
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

    char *message = set_message(ID_protocol, status);
    int size = get_procotol_length(ID_protocol);

    ESP_LOGI(TAG, "Largo %d\n", size);

    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

    sendto(sock, message, size, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    ESP_LOGI(TAG, "Se enviaron los datos");

    return 0;
}

void socket_udp(char ID_protocol, char status)
{
    while (udp_conn(ID_protocol, status) == 0)
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
        int32_t status[1] = {0};
        Write_NVS(status, 1);
        esp_deep_sleep(3000000);
        return;
    }
    ESP_LOGI(TAG, "Conectado al servidor %s:%d", SERVER_IP, SERVER_PORT);

    char ID_protocol;
    Read_NVS(&ID_protocol, 2);
    printf("ID Protocol: %d\n", ID_protocol);
    char status;
    Read_NVS(&status, 1);
    printf("Status: %d\n", status);
    char disc_time;
    Read_NVS(&disc_time, 7);
    printf("Disc Time: %d\n", disc_time);

    while (1)
    {
        char *message = set_message(ID_protocol, status);
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

        struct timeval timeout;
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

        char buffer[10];
        int rx_len = recv(sock, buffer, sizeof(buffer), 0);
        if (rx_len > 0)
        {
            buffer[rx_len] = '\0';
            ESP_LOGI(TAG, "Datos recibidos (%d): %s\n", rx_len, buffer);
            if (strcmp(buffer, "STOP") == 0)
            {
                int32_t status[1] = {0};
                Write_NVS(status, 1);
                printf("Se detiene la conexión por la RASP\n");
                close(sock);
                esp_deep_sleep(3000000);
            }
        }
        if (status == 22)
        {
            printf("Deep sleep\n");
            close(sock);
            esp_deep_sleep(disc_time * 1000000);

        }
    }

    // esp_deep_sleep_enable_timer_wakeup(60000000); // 10000000 us = 10 s

    // esp_deep_sleep_start();
}

void tcp_conf()
{
    int PORT;
    Read_NVS(&PORT, 8);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
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
        int32_t status[1] = {0};
        Write_NVS(status, 1);
        Write_NVS(status, 1);
        esp_deep_sleep(10000000);
        return;
    }
    ESP_LOGI(TAG, "Conectado al servidor %s:%d", SERVER_IP, PORT);

    char buffer[512];

    int rx_len = recv(sock, buffer, sizeof(buffer), 0);
    if (rx_len > 0)
    {
        buffer[rx_len] = '\0';
        ESP_LOGI(TAG, "Datos recibidos (%d): %s\n", rx_len, buffer);
        char *tokens = strtok(buffer, ";");

        char *status = tokens;
        tokens = strtok(NULL, ";");
        char status_int = (char)atoi(status);
        Write_NVS(status_int, 1);
        ESP_LOGI(TAG, "Status: %d", status_int);

        char *id_protocol = tokens;
        tokens = strtok(NULL, ";");
        int32_t id_protocol_int = (int32_t)atoi(id_protocol);
        Write_NVS(id_protocol_int, 2);
        ESP_LOGI(TAG, "ID Protocol: %ld", id_protocol_int);

        char *s_acc_sampling = tokens;
        tokens = strtok(NULL, ";");
        int acc_sampling = atoi(s_acc_sampling);
        Write_NVS(acc_sampling, 3);
        ESP_LOGI(TAG, "Acc Sampling: %d", acc_sampling);

        char *s_acc_sensibility = tokens;
        tokens = strtok(NULL, ";");
        int acc_sensibility = atoi(s_acc_sensibility);
        Write_NVS(acc_sensibility, 4);
        ESP_LOGI(TAG, "Acc Sensibility: %d", acc_sensibility);

        char *s_gyro_sampling = tokens;
        tokens = strtok(NULL, ";");
        int gyro_sampling = atoi(s_gyro_sampling);
        Write_NVS(gyro_sampling, 5);
        ESP_LOGI(TAG, "Gyro Sampling: %d", gyro_sampling);

        char *s_bme_sampling = tokens;
        tokens = strtok(NULL, ";");
        int bme_sampling = atoi(s_bme_sampling);
        Write_NVS(bme_sampling, 6);
        ESP_LOGI(TAG, "BME Sampling: %d", bme_sampling);

        char *s_disc_time = tokens;
        tokens = strtok(NULL, ";");
        int disc_time = atoi(s_disc_time);
        Write_NVS(disc_time, 7);
        ESP_LOGI(TAG, "Disc Time: %d", disc_time);

        char *s_tcp_port = tokens;
        tokens = strtok(NULL, ";");
        int tcp_port = atoi(s_tcp_port);
        Write_NVS(tcp_port, 8);
        ESP_LOGI(TAG, "TCP Port: %d", tcp_port);

        char *s_udp_port = tokens;
        tokens = strtok(NULL, ";");
        int udp_port = atoi(s_udp_port);
        Write_NVS(udp_port, 9);
        ESP_LOGI(TAG, "UDP Port: %d", udp_port);

        char *s_ip = tokens;
        tokens = strtok(NULL, ";");
        ESP_LOGI(TAG, "IP: %s", s_ip);

        char *s_ssid = tokens;
        tokens = strtok(NULL, ";");
        ESP_LOGI(TAG, "SSID: %s", s_ssid);

        char *s_pass = tokens;
        ESP_LOGI(TAG, "Pass: %s", s_pass);
    }

    esp_deep_sleep(10000000);
}