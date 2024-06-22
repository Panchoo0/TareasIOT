from PyQt5.QtWidgets import QApplication, QDialog
import threading
import random
from PyQt5 import QtCore, QtWidgets
import pyqtgraph as pg
from PyQt5 import uic


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
