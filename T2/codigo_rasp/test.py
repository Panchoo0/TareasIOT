from bleak import BleakScanner, BleakClient
import asyncio
import sys
from PyQt5 import uic
from PyQt5.QtWidgets import QApplication, QDialog
import pygatt
from tcp_server import socket_server

# ESP32 MAC ADDRESS 1 y 2
ESP32_1_MAC_ADDRESS = "C8:2E:18:F4:E6:16"
ESP32_2_MAC_ADDRESS = "4C:EB:D6:61:FE:D6"

# chars
CHARACTERISTICS = {
    "status": "0000ff01-0000-1000-8000-00805f9b34fb",
    "ID_protocol": "0000ff02-0000-1000-8000-00805f9b34fb",
    "BMI270_sampling": "0000ff03-0000-1000-8000-00805f9b34fb",
    "BMI270_acc_sensibility": "0000ff04-0000-1000-8000-00805f9b34fb",
    "BMI270_gyro_sensibility": "0000ff05-0000-1000-8000-00805f9b34fb",
    "BME688_sampling": "0000ff06-0000-1000-8000-00805f9b34fb",
    "discontinous_time": "0000ff07-0000-1000-8000-00805f9b34fb",
    "port_tcp": "0000ff08-0000-1000-8000-00805f9b34fb",
    "port_udp": "0000ff09-0000-1000-8000-00805f9b34fb",
    "host_ip_addr": "0000ff0A-0000-1000-8000-00805f9b34fb",
    "ssid": "0000ff0B-0000-1000-8000-00805f9b34fb",
    "pass": "0000ff0C-0000-1000-8000-00805f9b34fb"
}

# Parametros de configuracion
config_param = {
    "status" : 0x20,
    "ID_protocol" : 0x02,
    "BMI270_sampling" : 0x03,
    "BMI270_acc_sensibility" : 0x04,
    "BMI270_gyro_sensibility" : 0x05,
    "BME688_sampling" : 0x06,
    "discontinous_time" : 0x07,
    "port_tcp" : 0x08,
    "port_udp" : 0x09,
    "host_ip_addr" : 0x0A,
    "ssid" : 0x0B,
    "pass" : 0x0C,
}


def conf_status_0():
    adapter = pygatt.GATTToolBackend()
    ESP32_1_MAC_ADDRESS = "C8:2E:18:F4:E6:16"
    while True:
        try:
            adapter.start()
            print(f"Conectando a {ESP32_1_MAC_ADDRESS}...")
            device = adapter.connect(
                ESP32_1_MAC_ADDRESS, address_type=pygatt.BLEAddressType.random, timeout=15)
            break
        
        except pygatt.exceptions.NotConnectedError:
            print("No se pudo conectar al ESP32. Asegúrate de que el dispositivo está en modo de publicidad y dentro del alcance.")

    print("Conectado al ESP32")        
    adapter.stop()

def conf_status_20():
    pass


import asyncio
from bleak import BleakScanner, BleakClient

async def discover():
    # Con esto podemos ver los dispositivos que están disponibles
    scanner = BleakScanner()
    devices = await scanner.discover()
    for device in devices:
        print(device)
    return devices

async def connect(device_mac):
    # Con esto nos conectamos a un dispositivo
    async with BleakClient(device_mac) as client:
        connected = await client.is_connected()
        if connected:
            print(f"Conectado a {device_mac}")
        else:
            print(f"No se pudo conectar a {device_mac}")
        return client, connected
    

def convert_to_128bit_uuid(short_uuid):
    base_uuid = "00000000-0000-1000-8000-00805F9B34FB"
    short_uuid_hex = "{:04X}".format(short_uuid)
    res = base_uuid[:4] + short_uuid_hex + base_uuid[8:]
    print(f"UUID: {res}")
    return res


CHARACTERISTIC_UUID = convert_to_128bit_uuid(0xFF01)
CHARACTERISTIC_UUID_2 = convert_to_128bit_uuid(0xEE02)

async def main():
    index = 0
    try:
        index += 1
        print(f"Intento de conexión {index}")
        # await connect(ESP32_1_MAC_ADDRESS)
        # async with BleakClient(ESP32_1_MAC_ADDRESS) as client:
            # char_value = await client.read_gatt_char(CHARACTERISTIC_UUID_2)
            # print(f"Characteristic value (bytearray): {char_value}")

            # # Assuming the characteristic value is a single 4-byte integer (little-endian)
            # int_value = int.from_bytes(
            #     char_value, byteorder='little', signed=False)
            # print(f"Characteristic value (string): {char_value.decode('utf-8')}")
            # print(f"Characteristic value (hex): {char_value.hex()}")
            # print(f"Characteristic value (integer): {int_value}")

            # data = bytearray(b'\x05\x05')  # Example data to write
            # await client.write_gatt_char(CHARACTERISTIC_UUID, data)
            # print(f"Data written to characteristic {CHARACTERISTIC_UUID_2}")
            
            #
            # for key, char_uuid in CHARACTERISTICS.items():
            #     value = config_param[key]
            #     print(f"Setting characteristic {key} to {value}")
            #     bytearray_value = bytearray([value])
            #     await client.write_gatt_char(char_uuid, bytearray_value)
            #     print(f"Characteristic {key} value set to {value}")
            #     char_value = await client.read_gatt_char(char_uuid)
            #     print(f"Characteristic {key} value: {char_value}")

        socket_server()

    except KeyboardInterrupt:
        print("Programa terminado")
    except Exception as e:
        print(f"Error: {e}")
        print("No se pudo conectar")





if __name__ == "__main__":
   asyncio.run(main())