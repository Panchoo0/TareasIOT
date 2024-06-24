#include "lwip/sockets.h" // Para sockets
#include "esp_sleep.h"
#include "gen_data.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_func.h"
#include <unistd.h>

// Variables de WiFi
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
static const char *TAG = "WIFI";
static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;

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

void wifi_init_sta()
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

    char ssid_aux[20];
    Read_NVS_string(ssid_aux, 20, 11);
    char password_aux[20];
    Read_NVS_string(password_aux, 20, 12);

    // Set the specific fields
    strcpy((char *)wifi_config.sta.ssid, ssid_aux);
    strcpy((char *)wifi_config.sta.password, password_aux);
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
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", ssid_aux,
                 password_aux);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid_aux,
                 password_aux);
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

void socket_udp(char ID_protocol, char status)
{
    int PORT;
    Read_NVS(&PORT, 9);
    char IP[20];
    Read_NVS_string(IP, 20, 10);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &server_addr.sin_addr.s_addr);

    // Crear un socket
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Error al crear el socket");
        return ;
    }

    while (1)
    {
        char *message = set_message(ID_protocol, status);
        int size = get_procotol_length(ID_protocol);

        ESP_LOGI(TAG, "Largo %d", size);
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

        sendto(sock, message, size, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        ESP_LOGI(TAG, "Se enviaron los datos");

        char buffer[10];
        int rx_len = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
        if (rx_len > 0)
        {
            buffer[rx_len] = '\0';
            ESP_LOGI(TAG, "Datos recibidos (%d): %s", rx_len, buffer);
            if (strcmp(buffer, "STOP") == 0)
            {
                int32_t status[1] = {0};
                Write_NVS(status, 1);
                printf("Se detiene la conexión por la RASP");
                close(sock);
                esp_deep_sleep(3000000);
                
            }
        }
    }

}

void socket_tcp()
{
    int PORT;
    Read_NVS(&PORT, 8);
    char IP[20];
    Read_NVS_string(IP, 20, 10);

    ESP_LOGI(TAG, "IP: %s", IP);
    ESP_LOGI(TAG, "PORT: %d", PORT);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &server_addr.sin_addr.s_addr);

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
    ESP_LOGI(TAG, "Conectado al servidor %s:%d", IP, PORT);

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
        } else {
            printf("No se recibieron datos");
        }
        if (status == 22)
        {
            printf("Deep sleep\n");
            close(sock);
            esp_deep_sleep(disc_time * 1000000);
        }
        else
        {
            sleep(5); // Sleep for 5 seconds
        }
    }
}

void tcp_conf()
{
    int PORT;
    Read_NVS(&PORT, 8);
    char IP[20];
    Read_NVS_string(IP, 20, 10);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &server_addr.sin_addr.s_addr);

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
    ESP_LOGI(TAG, "Conectado al servidor %s:%d", IP, PORT);

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
        Write_NVS_string(s_ip, strlen(s_ip), 10);

        char *s_ssid = tokens;
        tokens = strtok(NULL, ";");
        ESP_LOGI(TAG, "SSID: %s", s_ssid);
        Write_NVS_string(s_ssid, strlen(s_ssid), 11);

        char *s_pass = tokens;
        ESP_LOGI(TAG, "Pass: %s", s_pass);
        Write_NVS_string(s_pass, strlen(s_pass), 12);
    }

    esp_deep_sleep(10000000);
}

void connect_to_wifi()
{
    wifi_init_sta();
    ESP_LOGI(TAG, "Conectado a WiFi!\n");
    srand(time(NULL));

    int32_t status[1];

    Read_NVS(status, 1);

    if (status[0] == 20)
    {
        tcp_conf();
    }
    else if (status[0] == 21 || status[0] == 22)
    {
        socket_tcp();
    } else if (status[0] == 23)
    {
        socket_udp(1, 23);
    }
    else
    {
        ESP_LOGE(TAG, "Error en el status");
    }
}