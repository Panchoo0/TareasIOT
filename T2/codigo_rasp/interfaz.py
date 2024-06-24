import asyncio
from bleak import BleakScanner, BleakClient
import sys
from PyQt5.QtWidgets import QApplication, QDialog, QGraphicsView
import threading
import random
from PyQt5 import QtCore, QtWidgets
from PyQt5.QtGui import QPen, QColor
import pyqtgraph as pg
from PyQt5 import uic
from qasync import QEventLoop, asyncSlot
from tcp_server import TCP_Server, tcp_bd, UDP_Server
import multiprocessing

ESP32_1_MAC_ADDRESS = "C8:2E:18:F4:E6:16"

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


def status_index_to_code(index):
    if index == 0:
        return 0
    elif index == 1:
        return 20
    elif index == 2:
        return 21
    elif index == 3:
        return 22
    elif index == 4:
        return 23
    elif index == 5:
        return 30
    elif index == 6:
        return 31


class ConfigDialog(QDialog):
    def __init__(self):
        super().__init__()
        uic.loadUi('iot_tarea2.ui', self)  # Load the .ui file
        self.setSignals()
        self.status = 0
        self.text_tcp_port.setText("1234")
        self.text_udp_port.setText("1235")
        self.text_host_ip.setText("192.168.1.83")
        self.text_ssid.setText("TeoyKala 2.4")
        self.text_pass.setText("208470701G")

        self.text_acc_sampling.setText("100")
        self.text_acc_sensibity.setText("2")
        self.text_gyro_sensibility.setText("250")
        self.textEdit_18.setText("1000")
        self.text_disc_time.setText("5")



    def setSignals(self):
        self.selec_10.currentIndexChanged.connect(self.leer)
        self.boton_configuracion_3.clicked.connect(self.discover)
        self.selec_esp.addItem("ESP1")
        self.selec_esp.addItem("ESP2")
        self.boton_configuracion.clicked.connect(self.configure)
        self.boton_inicio.clicked.connect(self.start_monitor)
        self.boton_detener.clicked.connect(self.stop_monitor)


    def leer(self):
        index = self.selec_10.currentIndex()
        texto = self.selec_10.itemText(index)
        print(texto)
        return texto

    def discover(self):
        print(self.selec_esp.currentText())

    def stop_monitor(self):
        print("stop")
        print(self.status)
        if self.status == 21 or self.status == 22:
            self.tcp_server.stop_server()
            # self.tcp_server_thread.join()
            self.tcp_server_thread = None
        if self.status == 23:
            self.udp_server.stop_server()
            # self.udp_server_thread.join()
            self.udp_server_thread = None

            

    # def initGraph(self):
    #     self.plot_widget = self.plot1 
    #     self.plot = self.plot_widget.addPlot()
    #     self.plot.showGrid(x=True, y=True)
    #     self.plot.setLabel('left', 'Value', '')
    #     self.plot.setLabel('bottom', 'Time', 's')
    #     self.curve = self.plot.plot(pen='y')
    #     self.data = []
    #     self.timestamps = []

    @asyncSlot()
    async def configure(self):
        acc_sampling = self.text_acc_sampling.toPlainText()
        acc_sensibility = self.text_acc_sensibity.toPlainText()
        gyro_sensibility = self.text_gyro_sensibility.toPlainText()
        bme688_sampling = self.textEdit_18.toPlainText()
        disc_time = self.text_disc_time.toPlainText()
        tcp_port = self.text_tcp_port.toPlainText()
        udp_port = self.text_udp_port.toPlainText()
        ip_addr = self.text_host_ip.toPlainText()
        ssid = self.text_ssid.toPlainText()
        password = self.text_pass.toPlainText()

        status = self.selec_10.currentIndex()
        status = status_index_to_code(status)
        print(f"status: {status}")

        id_protocol = self.selec_11.currentIndex() + 1

        if self.status == 0:
            try:
                async with BleakClient(ESP32_1_MAC_ADDRESS) as client:
                    await client.write_gatt_char(CHARACTERISTICS["status"], bytearray([status]))
                    await client.write_gatt_char(CHARACTERISTICS["ID_protocol"], bytearray([id_protocol]))
                    await client.write_gatt_char(CHARACTERISTICS["BMI270_sampling"], bytearray(int(acc_sampling).to_bytes(4)))
                    await client.write_gatt_char(CHARACTERISTICS["BMI270_acc_sensibility"], bytearray(int(acc_sensibility).to_bytes(4)))
                    await client.write_gatt_char(CHARACTERISTICS["BMI270_gyro_sensibility"], bytearray(int(gyro_sensibility).to_bytes(4)))
                    await client.write_gatt_char(CHARACTERISTICS["BME688_sampling"], bytearray(int(bme688_sampling).to_bytes(4)))
                    await client.write_gatt_char(CHARACTERISTICS["discontinous_time"], bytearray(int(disc_time).to_bytes(4)))
                    await client.write_gatt_char(CHARACTERISTICS["port_tcp"], bytearray(int(tcp_port).to_bytes(4)))
                    await client.write_gatt_char(CHARACTERISTICS["port_udp"], bytearray(int(udp_port).to_bytes(4)))
                    await client.write_gatt_char(CHARACTERISTICS["host_ip_addr"], bytearray(ip_addr, 'utf-8'))
                    await client.write_gatt_char(CHARACTERISTICS["ssid"], bytearray(ssid, 'utf-8'))
                    await client.write_gatt_char(CHARACTERISTICS["pass"], bytearray(password, 'utf-8'))
            except Exception as e:
                print(f"Error: {e}")


        elif self.status == 20:
            tcp_bd(acc_sampling, acc_sensibility, gyro_sensibility, bme688_sampling, disc_time, tcp_port, udp_port, ip_addr, ssid, password, status, id_protocol)

        print(f"1status: {status}")
        self.status = status
        print(f"2status: {status}")




    @asyncSlot()
    async def start_monitor(self):
        esp = self.selec_esp.currentText()
        ADDRESS = ""
        if esp == "ESP1":
            ADDRESS = "C8:2E:18:F4:E6:16"
        else:
            ADDRESS = "4C:EB:D6:61:FE:D6"
        acc_sampling = self.text_acc_sampling.toPlainText()
        acc_sensibility = self.text_acc_sensibity.toPlainText()
        gyro_sensibility = self.text_gyro_sensibility.toPlainText()
        bme688_sampling = self.textEdit_18.toPlainText()
        disc_time = self.text_disc_time.toPlainText()
        tcp_port = self.text_tcp_port.toPlainText()
        udp_port = self.text_udp_port.toPlainText()
        ip_addr = self.text_host_ip.toPlainText()
        ssid = self.text_ssid.toPlainText()
        password = self.text_pass.toPlainText()

        status = self.selec_10.currentIndex()
        status = status_index_to_code(status)

        id_protocol = self.selec_11.currentIndex() +1 

        selected_variable = self.selec_plot1.currentText().lower()

        print(f"selected var: {selected_variable}")
        try:
            # consola
            # data = await client.read_gatt_char(CHARACTERISTICS["status"])
            self.consola_1.setText("Status:")
            print("3Status: ", status)
            if status == 21 or status == 22:
                self.tcp_server = TCP_Server()
                self.tcp_server_thread = threading.Thread(
                    target=self.tcp_server.tcp_server)
                self.tcp_server_thread.start()
            if status == 23:
                self.udp_server = UDP_Server()
                self.udp_server_thread = threading.Thread(
                    target=self.udp_server.udp_server)
                self.udp_server_thread.start()
                

        except Exception as e:
            print(f"Error: {e}")

    # grafico
    def update_plot(self):
        self.scene.clear()
        pen = QPen(QColor(0, 255, 0))
        if len(self.data) > 1:
            for i in range(len(self.data) - 1):
                x1 = self.timestamps[i] - self.timestamps[0]
                y1 = self.data[i]
                x2 = self.timestamps[i + 1] - self.timestamps[0]
                y2 = self.data[i + 1]
                line = QGraphicsLineItem(QPointF(x1, y1), QPointF(x2, y2))
                line.setPen(pen)
                self.scene.addItem(line)
     
# if __name__ == "__main__":
#     app = QApplication(sys.argv)
#     dialog = ConfigDialog()
#     dialog.show()
#     sys.exit(app.exec_())
if __name__ == '__main__':
    app = QApplication(sys.argv)

    # Create and set the asyncio event loop
    loop = QEventLoop(app)
    asyncio.set_event_loop(loop)

    ex = ConfigDialog()
    ex.show()

    # Run the asyncio event loop with the PyQt5 event loop
    try:
        sys.exit(loop.run_forever())
    except KeyboardInterrupt:
        print("Keyboard interrupt received, exiting.")
        loop.stop()
        sys.exit(0)
