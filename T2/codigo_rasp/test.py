from bleak import BleakScanner, BleakClient
import asyncio
import sys
from PyQt5 import uic
from PyQt5.QtWidgets import QApplication, QDialog
import pygatt

# class ConfigDialog(QDialog):
#     def __init__(self):
#         super().__init__()
#         uic.loadUi('iot_tarea2.ui', self)  # Load the .ui file


# ESP32 MAC ADDRESS 1 y 2
ESP32_1_MAC_ADDRESS = "C8:2E:18:F4:E6:16"
ESP32_2_MAC_ADDRESS = "XX:XX:.."

# Parametros de configuracion
config_param = {
    "status" : 0x01,
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

    try:
        adapter.start()
        print(f"Conectando a {ESP32_1_MAC_ADDRESS}...")
        device = adapter.connect(
            ESP32_1_MAC_ADDRESS, address_type=pygatt.BLEAddressType.random, timeout=15)
    
    except pygatt.exceptions.NotConnectedError:
        print("No se pudo conectar al ESP32. Asegúrate de que el dispositivo está en modo de publicidad y dentro del alcance.")
        
    finally:
        adapter.stop()

def conf_status_20():
    pass

# conf_status_0()

# if __name__ == "__main__":
#     app = QApplication(sys.argv)
#     dialog = ConfigDialog()
#     dialog.show()
#     sys.exit(app.exec_())


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

async def main():
    devices = await discover()
    if devices:
        await connect("C8:2E:18:F4:E6:16")

if __name__ == "__main__":
   asyncio.run(main())