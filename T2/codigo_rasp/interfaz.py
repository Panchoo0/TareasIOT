from PyQt5.QtWidgets import QApplication, QDialog
import threading
import random
from PyQt5 import QtCore, QtWidgets
import pyqtgraph as pg
from PyQt5 import uic


class Ui_Dialog(object):

    def setupSensorSelectUi(self, Dialog):
        self.selec_12 = QtWidgets.QComboBox(Dialog)
        self.selec_12.setGeometry(QtCore.QRect(350, 160, 181, 31))
        self.selec_12.setStyleSheet("background-color: rgb(255, 255, 255);")
        self.selec_12.setObjectName("selec_12")
        self.selec_12.addItem("")
        self.selec_12.addItem("")
        self.selec_12.addItem("")

    def setupAccUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate

        # Labels
        self.label_32 = QtWidgets.QLabel(Dialog)
        self.label_32.setGeometry(QtCore.QRect(170, 100, 121, 21))
        self.label_32.setStyleSheet("color: rgb(0, 0, 0);\n"
                                    "\n"
                                    "")
        self.label_32.setObjectName("label_32")
        self.label_32.setText(_translate(
            "Dialog", "<html><head/><body><p><span style=\" text-decoration: underline;\">Acelerómetro</span></p></body></html>"))

        self.label_9 = QtWidgets.QLabel(Dialog)
        self.label_9.setGeometry(QtCore.QRect(120, 180, 81, 31))
        self.label_9.setObjectName("label_9")
        self.label_9.setText(_translate(
            "Dialog", "Frecuencia de \n muestreo 2"))

        self.label_7 = QtWidgets.QLabel(Dialog)
        self.label_7.setGeometry(QtCore.QRect(120, 130, 81, 31))
        self.label_7.setObjectName("label_7")
        self.label_7.setText(_translate("Dialog", "Sensibilidad 2"))

        # Selectores
        self.text_acc_sensibity = QtWidgets.QComboBox(Dialog)
        self.text_acc_sensibity.setGeometry(QtCore.QRect(210, 180, 104, 31))
        self.text_acc_sensibity.addItem("")
        self.text_acc_sensibity.addItem("")
        self.text_acc_sensibity.addItem("")
        self.text_acc_sensibity.addItem("")
        self.text_acc_sensibity.setObjectName("text_acc_sensibity")
        self.text_acc_sensibity.setItemText(0, _translate("Dialog", "200Hz"))
        self.text_acc_sensibity.setItemText(1, _translate("Dialog", "400Hz"))
        self.text_acc_sensibity.setItemText(2, _translate("Dialog", "800Hz"))
        self.text_acc_sensibity.setItemText(3, _translate("Dialog", "1600Hz"))

        self.comboBox_acc_sampling = QtWidgets.QComboBox(Dialog)
        self.comboBox_acc_sampling.setGeometry(QtCore.QRect(210, 130, 101, 31))
        self.comboBox_acc_sampling.setObjectName("comboBox_acc_sampling")
        self.comboBox_acc_sampling.addItem("")
        self.comboBox_acc_sampling.addItem("")
        self.comboBox_acc_sampling.addItem("")
        self.comboBox_acc_sampling.addItem("")
        self.comboBox_acc_sampling.setItemText(0, _translate("Dialog", "2g"))
        self.comboBox_acc_sampling.setItemText(1, _translate("Dialog", "4g"))
        self.comboBox_acc_sampling.setItemText(2, _translate("Dialog", "8g"))
        self.comboBox_acc_sampling.setItemText(3, _translate("Dialog", "16g"))

    def setupBMI270Ui(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        self.selec_13 = QtWidgets.QComboBox(Dialog)
        self.selec_13.setGeometry(QtCore.QRect(360, 300, 181, 31))
        self.selec_13.setStyleSheet("background-color: rgb(255, 255, 255);")
        self.selec_13.setObjectName("selec_13")
        self.selec_13.addItem("")
        self.selec_13.addItem("")
        self.selec_13.addItem("")
        self.selec_13.addItem("")
        self.selec_13.setItemText(
            0, _translate("Dialog", "Suspend Power Mode"))
        self.selec_13.setItemText(1, _translate("Dialog", "Lower Power Mode"))
        self.selec_13.setItemText(2, _translate("Dialog", "Normal Power Mode"))
        self.selec_13.setItemText(3, _translate(
            "Dialog", "Performance Power Mode"))

        self.label_31 = QtWidgets.QLabel(Dialog)
        self.label_31.setGeometry(QtCore.QRect(390, 270, 121, 21))
        self.label_31.setObjectName("label_31")
        self.label_31.setStyleSheet("color: rgb(0, 0, 0);\n"
                                    "\n"
                                    "")
        self.label_31.setText(_translate("Dialog", "Modo de Poder"))

        self.label_31.hide()
        self.selec_13.hide()

    def onBMI270Select(self):
        self.label_31.show()
        self.selec_13.show()
        self.label_31b.hide()
        self.selec_13b.hide()

    def onBMI688Select(self):
        self.label_31.hide()
        self.selec_13.hide()
        self.label_31b.show()
        self.selec_13b.show()

    def setupBMI688Ui(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        self.selec_13b = QtWidgets.QComboBox(Dialog)
        self.selec_13b.setGeometry(QtCore.QRect(360, 300, 181, 31))
        self.selec_13b.setStyleSheet("background-color: rgb(255, 255, 255);")
        self.selec_13b.setObjectName("selec_13")
        self.selec_13b.addItem("")
        self.selec_13b.addItem("")
        self.selec_13b.setItemText(2, _translate("Dialog", "Forzado"))
        self.selec_13b.setItemText(3, _translate("Dialog", "Paralelo"))

        self.label_31b = QtWidgets.QLabel(Dialog)
        self.label_31b.setGeometry(QtCore.QRect(390, 270, 121, 21))
        self.label_31b.setObjectName("label_31")
        self.label_31b.setStyleSheet("color: rgb(0, 0, 0);\n"
                                     "\n"
                                     "")
        self.label_31b.setText(_translate("Dialog", "Modo de Funcionamiento"))
        self.label_31b.hide()
        self.selec_13b.hide()

    def setupGyroUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        # Labels
        self.label_33 = QtWidgets.QLabel(Dialog)
        self.label_33.setGeometry(QtCore.QRect(170, 220, 121, 21))
        self.label_33.setStyleSheet("color: rgb(0, 0, 0);\n"
                                    "\n"
                                    "")
        self.label_33.setObjectName("label_33")
        self.label_33.setText(_translate(
            "Dialog", "<html><head/><body><p><span style=\" text-decoration: underline;\">Giroscopio</span></p></body></html>"))

        self.label_8 = QtWidgets.QLabel(Dialog)
        self.label_8.setGeometry(QtCore.QRect(120, 250, 81, 31))
        self.label_8.setObjectName("label_8")
        self.label_10 = QtWidgets.QLabel(Dialog)
        self.label_10.setGeometry(QtCore.QRect(120, 300, 81, 31))
        self.label_10.setObjectName("label_10")

        self.label_8.setText(_translate("Dialog", "Sensibilidad"))
        self.label_10.setText(_translate("Dialog", "Frecuencia de \n"
                                         " muestro"))

        # Selectores
        self.text_acc_sensibity_2 = QtWidgets.QComboBox(Dialog)
        self.text_acc_sensibity_2.setGeometry(QtCore.QRect(210, 300, 104, 31))
        self.text_acc_sensibity_2.setObjectName("text_acc_sensibity_2")
        self.text_acc_sensibity_2.addItem("")
        self.text_acc_sensibity_2.addItem("")
        self.text_acc_sensibity_2.addItem("")
        self.text_acc_sensibity_2.addItem("")
        self.text_acc_sensibity_2.setItemText(0, _translate("Dialog", "200Hz"))
        self.text_acc_sensibity_2.setItemText(1, _translate("Dialog", "400Hz"))
        self.text_acc_sensibity_2.setItemText(2, _translate("Dialog", "800Hz"))
        self.text_acc_sensibity_2.setItemText(
            3, _translate("Dialog", "1600Hz"))

        self.text_acc_sampling_2 = QtWidgets.QComboBox(Dialog)
        self.text_acc_sampling_2.setGeometry(QtCore.QRect(210, 250, 104, 31))
        self.text_acc_sampling_2.setObjectName("text_acc_sampling_2")
        self.text_acc_sampling_2.addItem("")
        self.text_acc_sampling_2.addItem("")
        self.text_acc_sampling_2.addItem("")
        self.text_acc_sampling_2.addItem("")
        self.text_acc_sampling_2.setItemText(0, _translate("Dialog", "0"))
        self.text_acc_sampling_2.setItemText(1, _translate("Dialog", "1"))
        self.text_acc_sampling_2.setItemText(2, _translate("Dialog", "2"))
        self.text_acc_sampling_2.setItemText(3, _translate("Dialog", "3"))

    def setupUi(self, Dialog):
        Dialog.setObjectName("Dialog")
        Dialog.resize(774, 836)
        self.label_30 = QtWidgets.QLabel(Dialog)
        self.label_30.setGeometry(QtCore.QRect(350, 130, 101, 21))
        self.label_30.setStyleSheet("color: rgb(0, 0, 0);\n"
                                    "\n"
                                    "")
        self.label_30.setObjectName("label_30")
        self.progressBar = QtWidgets.QProgressBar(Dialog)
        self.progressBar.setGeometry(QtCore.QRect(460, 130, 118, 23))
        self.progressBar.setProperty("value", 0)
        self.progressBar.setObjectName("progressBar")

        self.setupSensorSelectUi(Dialog)
        self.setupAccUi(Dialog)
        self.setupGyroUi(Dialog)
        self.setupBMI270Ui(Dialog)
        self.setupBMI688Ui(Dialog)

        # Titulo conf sensor
        self.label_2 = QtWidgets.QLabel(Dialog)
        self.label_2.setGeometry(QtCore.QRect(350, 40, 71, 41))
        self.label_2.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.label_2.setFrameShape(QtWidgets.QFrame.Box)
        self.label_2.setAlignment(QtCore.Qt.AlignCenter)
        self.label_2.setObjectName("label_2")

        self.Plot1 = pg.PlotWidget(Dialog)
        self.Plot1.setGeometry(QtCore.QRect(60, 420, 291, 181))
        self.Plot1.setFrameShape(QtWidgets.QFrame.Box)
        self.Plot1.setFrameShadow(QtWidgets.QFrame.Plain)
        self.Plot1.setObjectName("Plot1")
        self.curve11 = self.Plot1.plot(pen="b")
        self.curve12 = self.Plot1.plot(pen="r")
        self.curve13 = self.Plot1.plot(pen="g")

        self.Plot2 = pg.PlotWidget(Dialog)
        self.Plot2.setGeometry(QtCore.QRect(390, 420, 291, 181))
        self.Plot2.setFrameShape(QtWidgets.QFrame.Box)
        self.Plot2.setFrameShadow(QtWidgets.QFrame.Plain)
        self.Plot2.setObjectName("Plot2")
        self.curve21 = self.Plot2.plot(pen="b")
        self.curve22 = self.Plot2.plot(pen="r")
        self.curve23 = self.Plot2.plot(pen="g")

        self.Plot3 = pg.PlotWidget(Dialog)
        self.Plot3.setGeometry(QtCore.QRect(60, 640, 291, 181))
        self.Plot3.setFrameShape(QtWidgets.QFrame.Box)
        self.Plot3.setFrameShadow(QtWidgets.QFrame.Plain)
        self.Plot3.setObjectName("Plot3")
        self.curve31 = self.Plot3.plot(pen="b")
        self.curve32 = self.Plot3.plot(pen="r")
        self.curve33 = self.Plot3.plot(pen="g")

        self.Plot4 = pg.PlotWidget(Dialog)
        self.Plot4.setGeometry(QtCore.QRect(390, 640, 291, 181))
        self.Plot4.setFrameShape(QtWidgets.QFrame.Box)
        self.Plot4.setFrameShadow(QtWidgets.QFrame.Plain)
        self.Plot4.setObjectName("Plot4")
        self.curve41 = self.Plot4.plot(pen="b")
        self.curve42 = self.Plot4.plot(pen="r")
        self.curve43 = self.Plot4.plot(pen="g")

        self.label_3 = QtWidgets.QLabel(Dialog)
        self.label_3.setGeometry(QtCore.QRect(120, 390, 151, 21))
        self.label_3.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.label_3.setFrameShape(QtWidgets.QFrame.Box)
        self.label_3.setAlignment(QtCore.Qt.AlignCenter)
        self.label_3.setObjectName("label_3")
        self.label_4 = QtWidgets.QLabel(Dialog)
        self.label_4.setGeometry(QtCore.QRect(440, 390, 151, 21))
        self.label_4.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.label_4.setFrameShape(QtWidgets.QFrame.Box)
        self.label_4.setAlignment(QtCore.Qt.AlignCenter)
        self.label_4.setObjectName("label_4")
        self.label_5 = QtWidgets.QLabel(Dialog)
        self.label_5.setGeometry(QtCore.QRect(120, 610, 151, 21))
        self.label_5.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.label_5.setFrameShape(QtWidgets.QFrame.Box)
        self.label_5.setAlignment(QtCore.Qt.AlignCenter)
        self.label_5.setObjectName("label_5")
        self.label_6 = QtWidgets.QLabel(Dialog)
        self.label_6.setGeometry(QtCore.QRect(440, 610, 151, 21))
        self.label_6.setLayoutDirection(QtCore.Qt.LeftToRight)
        self.label_6.setFrameShape(QtWidgets.QFrame.Box)
        self.label_6.setAlignment(QtCore.Qt.AlignCenter)
        self.label_6.setObjectName("label_6")
        self.pushButton = QtWidgets.QPushButton(Dialog)
        self.pushButton.setGeometry(QtCore.QRect(550, 160, 141, 31))
        self.pushButton.setObjectName("pushButton")
        self.pushButton_2 = QtWidgets.QPushButton(Dialog)
        self.pushButton_2.setGeometry(QtCore.QRect(320, 370, 101, 41))
        self.pushButton_2.setObjectName("pushButton_2")

        self.retranslateUi(Dialog)
        QtCore.QMetaObject.connectSlotsByName(Dialog)

    def retranslateUi(self, Dialog):
        _translate = QtCore.QCoreApplication.translate
        Dialog.setWindowTitle(_translate("Dialog", "UI Sensores"))
        self.label_30.setText(_translate(
            "Dialog", "<html><head/><body><p align=\"center\"><span style=\" text-decoration: underline;\">Sensor activo</span></p></body></html>"))
        self.selec_12.setItemText(0, _translate("Dialog", "<Ninguno>"))
        self.selec_12.setItemText(1, _translate("Dialog", "BMI270"))
        self.selec_12.setItemText(2, _translate("Dialog", "BMI688"))

        self.label_2.setText(_translate("Dialog", "Configuracion \n"
                                        " Sensor"))

        self.label_3.setText(_translate("Dialog", "Datos 1: Aceleración"))
        self.label_4.setText(_translate("Dialog", "Datos 2: RMS"))
        self.label_5.setText(_translate("Dialog", "Datos 3: FFT"))
        self.label_6.setText(_translate("Dialog", "Datos 4: Peaks"))
        self.pushButton.setText(_translate("Dialog", "Iniciar configuración"))
        self.pushButton_2.setText(_translate("Dialog", "Iniciar captación \n"
                                             " de datos"))


class Controller:

    def __init__(self, parent, app):
        self.ui = Ui_Dialog()
        self.parent = parent
        self.app = app


    def setSignals(self):
        self.ui.selec_12.currentIndexChanged.connect(self.leerModoOperacion)
        self.ui.pushButton.clicked.connect(self.leerConfiguracion)
        self.ui.pushButton_2.clicked.connect(self.onRead)
        self.parent.finished.connect(self.closeEvent)

    def leerConfiguracion(self):
        conf = dict()
        conf['AccSamp'] = self.ui.comboBox_acc_sampling.currentIndex()+1
        conf['AccSen'] = self.ui.text_acc_sensibity.currentIndex()+1
        conf['Modo'] = self.ui.selec_13.currentIndex()+1
        return conf

    def leerModoOperacion(self):
        index = self.ui.selec_12.currentIndex()
        texto = self.ui.selec_12.itemText(index)
        return texto

    def criticalError(self):
        popup = QtWidgets.QMessageBox(parent=self.parent)
        popup.setWindowTitle('Error Critico')
        popup.setIcon(QtWidgets.QMessageBox.Icon.Critical)
        popup.exec()
        return


# if __name__ == "__main__":
#     import sys
#     app = QtWidgets.QApplication(sys.argv)
#     Dialog = QtWidgets.QDialog()
#     cont = Controller(parent=Dialog, app=app)
#     ui = cont.ui
#     ui.setupUi(Dialog)
#     Dialog.show()
#     cont.setSignals()
#     sys.exit(app.exec_())



import sys
from PyQt5.QtWidgets import QApplication, QDialog

class ConfigDialog(QDialog):
    def __init__(self):
        super().__init__()
        uic.loadUi('iot_tarea2.ui', self)  # Load the .ui file
        self.setSignals()

    def setSignals(self):
        self.selec_10.currentIndexChanged.connect(self.leer)

    def leer(self):
        index = self.selec_10.currentIndex()
        texto = self.selec_10.itemText(index)
        print(texto)
        return texto

if __name__ == "__main__":
    app = QApplication(sys.argv)
    dialog = ConfigDialog()
    dialog.show()
    sys.exit(app.exec_())
