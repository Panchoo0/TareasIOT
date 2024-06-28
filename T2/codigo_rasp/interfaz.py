import asyncio
from bleak import BleakScanner, BleakClient
import sys
from PyQt5.QtWidgets import QApplication, QDialog, QGraphicsView, QGraphicsScene, QGraphicsLineItem
from PyQt5.QtCore import Qt, QRectF, QPointF
import threading
import random
from PyQt5 import QtCore, QtWidgets
from PyQt5.QtGui import QPen, QColor
import pyqtgraph as pg
from PyQt5 import uic
from qasync import QEventLoop, asyncSlot
from tcp_server import TCP_Server, tcp_bd, UDP_Server
import multiprocessing
from tcp_server import parse_data

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
    "pass": "0000ff0C-0000-1000-8000-00805f9b34fb",
    'values': "0000ff0D-0000-1000-8000-00805f9b34fb"
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
        uic.loadUi('design.ui', self)  # Load the .ui file
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
        self.batt_lvl = []
        self.temp = []
        self.pres = []
        self.hum = []
        self.co = []
        self.rms = []
        self.amp_x = []
        self.freq_x = []
        self.amp_y = []
        self.freq_y = []
        self.amp_z = []
        self.freq_z = []

        self.plot_1_widget = pg.PlotWidget()
        layout_1 = QtWidgets.QVBoxLayout()
        layout_1.addWidget(self.plot_1_widget)
        self.plot1.setLayout(layout_1)

        self.plot_2_widget = pg.PlotWidget()
        layout_2 = QtWidgets.QVBoxLayout()
        layout_2.addWidget(self.plot_2_widget)
        self.plot2.setLayout(layout_2)

        self.plot_3_widget = pg.PlotWidget()
        layout_3 = QtWidgets.QVBoxLayout()
        layout_3.addWidget(self.plot_3_widget)
        self.plot3.setLayout(layout_3)

        self.client = None
        self.devices = []

    def update_data(self, data):
        print(data)
        self.batt_lvl.append(data['batt_lvl'])
        if 'temp' in data:
            self.temp.append(data['temp'])
        if 'press' in data:
            self.pres.append(data['press'])
        if 'hum' in data:
            self.hum.append(data['hum'])
        if 'co' in data:
            self.co.append(data['co'])
        if 'rms' in data:
            self.rms.append(data['rms'])
        if 'amp_x' in data:
            self.amp_x.append(data['amp_x'])
        if 'freq_x' in data:
            self.freq_x.append(data['freq_x'])
        if 'amp_y' in data:
            self.amp_y.append(data['amp_y'])
        if 'freq_y' in data:
            self.freq_y.append(data['freq_y'])
        if 'amp_z' in data:
            self.amp_z.append(data['amp_z'])
        if 'freq_z' in data:
            self.freq_z.append(data['freq_z'])

        self.update_plot_1()
        self.update_plot_2()
        self.update_plot_3()

    def setSignals(self):
        self.boton_configuracion_3.clicked.connect(self.discover)
        self.boton_configuracion.clicked.connect(self.configure)
        self.boton_inicio.clicked.connect(self.start_monitor)
        self.boton_detener.clicked.connect(self.stop_monitor)
        self.boton_graficar.clicked.connect(self.initGraph)

    @asyncSlot()
    async def discover(self):
        # Con esto podemos ver los dispositivos que están disponibles
        self.consola_1.setText("Buscando dispositivos...")
        scanner = BleakScanner()
        devices = await scanner.discover()
        self.devices = []
        self.selec_esp.clear()
        for device in devices:
            print(device)
            if device.name is None:
                continue
            if 'ESP' in device.name:
                self.devices.append(device)
                self.selec_esp.addItem(device.name)
        self.consola_1.setText("Dispositivos encontrados")

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

    def initGraph(self):
        # self.plot_widget = self.plot1
        # self.plot = self.plot_widget.addPlot()
        # self.plot.showGrid(x=True, y=True)
        # self.plot.setLabel('left', 'Value', '')
        # self.plot.setLabel('bottom', 'Time', 's')
        # self.curve = self.plot.plot(pen='y')
        # self.data = []
        # self.timestamps = []
        print(self.plot1)

    @asyncSlot()
    async def connect(self):
        tries = 1
        esp = self.selec_esp.currentText()
        ADDRESS = ""
        for device in self.devices:
            if device.name == esp:
                ADDRESS = device.address
                break
        while True:
            try:
                self.consola_1.setText(
                    f"Conectando...\nIntento {tries}")
                async with BleakClient(ADDRESS) as client:
                    self.client = client
                    await client.is_connected()
                    print("Connected to ESP32", ADDRESS)
                    return client
            except Exception as e:
                print(f"Error: {e}")
                tries += 1

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

        id_protocol = self.selec_11.currentIndex() + 1

        esp = self.selec_esp.currentText()
        ADDRESS = ""
        for device in self.devices:
            if device.name == esp:
                ADDRESS = device.address
                break
        self.consola_1.setText("Conectando...")
        if self.status == 0 or self.status == 30 or self.status == 31:
            try:
                if self.client is None:
                    client = await self.connect()
                async with self.client as client:
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
            tcp_bd(acc_sampling, acc_sensibility, gyro_sensibility, bme688_sampling,
                   disc_time, tcp_port, udp_port, ip_addr, ssid, password, status, id_protocol)

        self.status = status
        self.consola_1.setText("Configuración enviada")

    def notify_callback(self, handle, value):
       print(f"value: {value}")
       data = parse_data(value)
       print(f"Parsed data: {data}")
       self.update_data(data)
       

    @asyncSlot()
    async def start_monitor(self):
        status = self.selec_10.currentIndex()
        status = status_index_to_code(status)

        try:
            self.consola_1.setText("Iniciando...")
            if status == 21 or status == 22:
                self.tcp_server = TCP_Server()
                self.tcp_server.update_data.connect(self.update_data)
                self.tcp_server.start()
                self.consola_1.setText("Iniciado")

            if status == 23:
                self.udp_server = UDP_Server()
                self.udp_server_thread = threading.Thread(
                    target=self.udp_server.udp_server)
                self.udp_server_thread.start()
                self.consola_1.setText("Iniciado")

            if status == 30:
                try:
                    async with self.client as client:
                        await client.start_notify(CHARACTERISTICS['values'], self.notify_callback)
                        self.consola_1.setText("Notificaciones activadas")
                        while True:
                            await asyncio.sleep(1)
                    self.consola_1.setText("Notificaciones activadas")
                except Exception as e:
                    print(f"Error: {e}")
                    await self.connect()
                    async with self.client as client:
                        await client.start_notify(CHARACTERISTICS['values'], self.notify_callback)
                        self.consola_1.setText("Notificaciones activadas")
                        while True:
                            await asyncio.sleep(1)
        
            if status == 31:
                while True:
                    try:
                        await self.connect()
                        async with self.client as client:
                            await client.start_notify(CHARACTERISTICS['values'], self.notify_callback)
                            self.consola_1.setText("Notificaciones activadas")
                            tries = 1
                            while tries <= 3:
                                await asyncio.sleep(0.1)
                                tries += 1
                    except KeyboardInterrupt:
                        print("Cerrando conexión")
                        break
                    except Exception as e:
                        print(f"Error: {e}")
                        await asyncio.sleep(0.1)


        except Exception as e:
            print(f"Error: {e}")

    def vars(self, var_sel):
        if var_sel == "Temperatura":
            return self.temp
        elif var_sel == "humedad":
            return self.hum
        elif var_sel == "presión":
            return self.press
        elif var_sel == "co":
            return self.co
        elif var_sel == "Amp_x":
            return self.amp_x
        elif var_sel == "Amp_y":
            return self.amp_y
        elif var_sel == "Amp_z":
            return self.amp_z
        elif var_sel == "Freq_x":
            return self.freq_x
        elif var_sel == "Freq_y":
            return self.freq_y
        elif var_sel == "Freq_z":
            return self.freq_z
        elif var_sel == "RMS":
            return self.rms
        elif var_sel == "batt_lvl":
            return self.batt_lvl
        pass
    
    
    # grafico 1
    def update_plot_1(self):
        selected_variable_plot1 = self.selec_plot1.currentText()
        # self.scene.clear()
        self.plot_1_widget.clear()
        pen = QPen(QColor(0, 255, 0))
        #data = self.batt_lvl
        data = self.vars(selected_variable_plot1)
        print(data)
        if len(data) > 0:
            x = list(range(len(data)))
            self.plot_1_widget.plot(x, data, pen=pg.mkPen(color=(0, 255, 0)))

    # grafico 2
    def update_plot_2(self):
        selected_variable_plot2 = self.selec_plot2.currentText()
        # self.scene.clear()
        self.plot_2_widget.clear()
        pen = QPen(QColor(0, 255, 0))
        #data = self.batt_lvl
        data = self.vars(selected_variable_plot2)
        print(data)
        if len(data) > 0:
            x = list(range(len(data)))
            self.plot_2_widget.plot(x, data, pen=pg.mkPen(color=(0, 255, 0)))

    # grafico 3
    def update_plot_3(self):
        selected_variable_plot3 = self.selec_plot3.currentText()
        # self.scene.clear()
        self.plot_3_widget.clear()
        pen = QPen(QColor(0, 255, 0))
        #data = self.batt_lvl
        data = self.vars(selected_variable_plot3)
        print(data)
        if len(data) > 0:
            x = list(range(len(data)))
            self.plot_3_widget.plot(x, data, pen=pg.mkPen(color=(0, 255, 0)))

    def update_console4(self):
        self.consola_4.setText("Status:")

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
