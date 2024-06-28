#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <esp_system.h>
#include "esp_mac.h"
#include <time.h>

typedef unsigned char u_char;

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

void acc(short *data)
{
    for (int i = 0; i < 2000; i++)
    {
        float num = gen_rand_float(32, -16);
        data[i] = *((short *)&num);
    }
}

void rgyro(short *data)
{
    for (int i = 0; i < 2000; i++)
    {
        float num = gen_rand_float(2000, -1000);
        data[i] = *((short *)&num);
    }
}

void mac(uint8_t *base_mac_addr)
{
    esp_read_mac(base_mac_addr, ESP_MAC_WIFI_STA);
}

void set_headers(char *headers, char ID_protocol, char status)
{
    headers[0] = ID_protocol;
    headers[1] = status;
    headers[2] = 0; // Largo del mensaje
    headers[3] = 0; // Largo del mensaje
    printf("Status: %d\n", status);
    printf("ID protocol: %d\n", ID_protocol);
}

void set_protocol_1(char *message, char ID_protocol, char status)
{
    set_headers(message, ID_protocol, status);
    message[2] = 0;                    // Tamaño del mensaje
    message[3] = 9;                    // Tamano del mensaje
    message[4] = (char)gen_batt_lvl(); // batt lvl

    // Timestamp
    message[5] = 0;
    message[6] = 0;
    message[7] = 0;
    message[8] = 0;
    printf("Batt lvl: %d\n", message[4]);
}

void set_protocol_2(char *message, char ID_protocol, char status)
{
    set_protocol_1(message, ID_protocol, status);
    message[2] = 0;  // Tamaño del mensaje
    message[3] = 19; // Tamano del mensaje

    message[9] = (char)gen_tmp(); // temp
    printf("Temperatura: %d\n", message[9]);
    int press = gen_pres();
    printf("Presion: %i\n", press);

    message[10] = (char)(press >> 24 & 0xFF);
    message[11] = (char)(press >> 16 & 0xFF);
    message[12] = (char)(press >> 8 & 0xFF);
    message[13] = (char)(press & 0xFF);

    message[14] = (char)gen_hum(); // hum
    printf("Humedad: %d\n", message[14]);

    float fco = gen_co();
    int co = *((int *)&fco);
    message[15] = (char)(co >> 24 & 0xFF);
    message[16] = (char)(co >> 16 & 0xFF);
    message[17] = (char)(co >> 8 & 0xFF);
    message[18] = (char)(co & 0xFF);
    printf("CO: %f\n", fco);
}

void set_protocol_3(char *message, char ID_protocol, char status)
{
    set_protocol_2(message, ID_protocol, status);

    message[2] = 0;  // Tamaño del mensaje
    message[3] = 23; // Tamano del mensaje

    int rms = 0; // Agregar RMS

    message[19] = (char)(rms >> 24 & 0xFF);
    message[20] = (char)(rms >> 16 & 0xFF);
    message[21] = (char)(rms >> 8 & 0xFF);
    message[22] = (char)(rms & 0xFF);
}

void set_protocol_4(char *message, char ID_protocol, char status)
{
    set_protocol_3(message, ID_protocol, status);

    message[2] = 0;  // Tamaño del mensaje
    message[3] = 47; // Tamano del mensaje

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

    message[23] = (char)(ampx >> 24 & 0xFF);
    message[24] = (char)(ampx >> 16 & 0xFF);
    message[25] = (char)(ampx >> 8 & 0xFF);
    message[26] = (char)(ampx & 0xFF);

    message[27] = (char)(freqx >> 24 & 0xFF);
    message[28] = (char)(freqx >> 16 & 0xFF);
    message[29] = (char)(freqx >> 8 & 0xFF);
    message[30] = (char)(freqx & 0xFF);

    message[31] = (char)(ampy >> 24 & 0xFF);
    message[32] = (char)(ampy >> 16 & 0xFF);
    message[33] = (char)(ampy >> 8 & 0xFF);
    message[34] = (char)(ampy & 0xFF);

    message[35] = (char)(freqy >> 24 & 0xFF);
    message[36] = (char)(freqy >> 16 & 0xFF);
    message[37] = (char)(freqy >> 8 & 0xFF);
    message[38] = (char)(freqy & 0xFF);

    message[39] = (char)(ampz >> 24 & 0xFF);
    message[40] = (char)(ampz >> 16 & 0xFF);
    message[41] = (char)(ampz >> 8 & 0xFF);
    message[42] = (char)(ampz & 0xFF);

    message[43] = (char)(freqz >> 24 & 0xFF);
    message[44] = (char)(freqz >> 16 & 0xFF);
    message[45] = (char)(freqz >> 8 & 0xFF);
    message[46] = (char)(freqz & 0xFF);
}

void set_protocol_5(char *message, char ID_protocol, char status)
{
    set_protocol_2(message, ID_protocol, status);

    int size = 19219;
    message[2] = (char)(size >> 8 & 0xFF);
    message[3] = (char)(size & 0xFF);

    short *acc_x = malloc(2000 * sizeof(short));
    short *acc_y = malloc(2000 * sizeof(short));
    short *acc_z = malloc(2000 * sizeof(short));

    acc(acc_x);
    acc(acc_y);
    acc(acc_z);

    short *gyro_x = malloc(2000 * sizeof(short));
    short *gyro_y = malloc(2000 * sizeof(short));
    short *gyro_z = malloc(2000 * sizeof(short));

    rgyro(gyro_x);
    rgyro(gyro_y);
    rgyro(gyro_z);

    for (int i = 0; i < 2000; i++)
    {
        message[19 + (i * 12)] = (char)(acc_x[i] >> 8 & 0xFF);
        message[20 + (i * 12)] = (char)(acc_x[i] & 0xFF);

        message[21 + (i * 12)] = (char)(acc_y[i] >> 8 & 0xFF);
        message[22 + (i * 12)] = (char)(acc_y[i] & 0xFF);

        message[23 + (i * 12)] = (char)(acc_z[i] >> 8 & 0xFF);
        message[24 + (i * 12)] = (char)(acc_z[i] & 0xFF);

        message[25 + (i * 12)] = (char)(gyro_x[i] >> 8 & 0xFF);
        message[26 + (i * 12)] = (char)(gyro_x[i] & 0xFF);

        message[27 + (i * 12)] = (char)(gyro_y[i] >> 8 & 0xFF);
        message[28 + (i * 12)] = (char)(gyro_y[i] & 0xFF);

        message[29 + (i * 12)] = (char)(gyro_z[i] >> 8 & 0xFF);
        message[30 + (i * 12)] = (char)(gyro_z[i] & 0xFF);
    }

    free(acc_x);
    free(acc_y);
    free(acc_z);
    free(gyro_x);
    free(gyro_y);
    free(gyro_z);
}

int get_procotol_length(char ID_protocol)
{
    if (ID_protocol == 1)
    {
        return 9;
    }
    else if (ID_protocol == 2)
    {
        return 19;
    }
    else if (ID_protocol == 3)
    {
        return 23;
    }
    else if (ID_protocol == 4)
    {
        return 47;
    }
    else if (ID_protocol == 5)
    {
        return 19219;
    }
    return 0;
}

char *set_message(char ID_protocol, char status)
{
    srand(time(NULL));
    char *message = NULL;
    if (ID_protocol == 1)
    {
        message = malloc(9 * sizeof(char));
        set_protocol_1(message, ID_protocol, status);
    }
    else if (ID_protocol == 2)
    {
        message = malloc(19 * sizeof(char));
        set_protocol_2(message, ID_protocol, status);
    }
    else if (ID_protocol == 3)
    {
        message = malloc(23 * sizeof(char));
        set_protocol_3(message, ID_protocol, status);
    }
    else if (ID_protocol == 4)
    {
        message = malloc(47 * sizeof(char));
        set_protocol_4(message, ID_protocol, status);
    }
    else if (ID_protocol == 5)
    {
        message = malloc(19219 * sizeof(char));
        set_protocol_5(message, ID_protocol, status);
    }
    return message;
}
