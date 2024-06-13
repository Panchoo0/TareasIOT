import sys
from PyQt5 import uic
from PyQt5.QtWidgets import QApplication, QDialog
import pygatt

# class ConfigDialog(QDialog):
#     def __init__(self):
#         super().__init__()
#         uic.loadUi('iot_tarea2.ui', self)  # Load the .ui file


# ESP32 MAC ADDRESS 1 y 2
ESP32_1_MAC_ADDRESS = "c8:2e:18:f4:e6:16"
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

    try:
        adapter.start()
        device = adapter.connect(ESP32_1_MAC_ADDRESS)
        print("hola me conecte")
        
    finally:
        adapter.stop()

def conf_status_20():
    pass

conf_status_20()

# if __name__ == "__main__":
#     app = QApplication(sys.argv)
#     dialog = ConfigDialog()
#     dialog.show()
#     sys.exit(app.exec_())

    
