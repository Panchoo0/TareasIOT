from bleak import BleakScanner, BleakClient
import asyncio
import sys
from PyQt5 import uic
from PyQt5.QtWidgets import QApplication, QDialog
import pygatt

# ESP32 MAC ADDRESS 1 y 2
ESP32_1_MAC_ADDRESS = "C8:2E:18:F4:E6:16"
ESP32_2_MAC_ADDRESS = "4C:EB:D6:61:FE:D6"


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


CHARACTERISTIC_UUID = convert_to_128bit_uuid(0xFF0D)
CHARACTERISTIC_UUID_2 = convert_to_128bit_uuid(0xEE02)


from tcp_server import parse_data

def notify_callback(handle, value):
       expected_data = b"CHK_DATA"
       data = parse_data(value)
       print(f"Parsed data: {data}")
       if expected_data in value:
           print("Notification received - Checking data")


async def main():
    async with BleakClient(ESP32_1_MAC_ADDRESS) as client:
        connected = await client.is_connected()
        print("Conectado")
        await client.start_notify(CHARACTERISTIC_UUID, notify_callback)
        print("Notificaciones activadas")
        while True:
            await asyncio.sleep(1)


if __name__ == "__main__":
   asyncio.run(discover())