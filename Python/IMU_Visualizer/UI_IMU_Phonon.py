# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_IMU_Phonon.ui'
#
# Created: Wed Apr 20 09:45:43 2011
#      by: PyQt4 UI code generator 4.7.5
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_IMU_Phonon(object):
    def setupUi(self, IMU_Phonon):
        IMU_Phonon.setObjectName(_fromUtf8("IMU_Phonon"))
        IMU_Phonon.resize(1027, 904)
        self.centralwidget = QtGui.QWidget(IMU_Phonon)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.gridLayout_2 = QtGui.QGridLayout(self.centralwidget)
        self.gridLayout_2.setObjectName(_fromUtf8("gridLayout_2"))
        self.verticalLayout_2 = QtGui.QVBoxLayout()
        self.verticalLayout_2.setSpacing(-1)
        self.verticalLayout_2.setSizeConstraint(QtGui.QLayout.SetDefaultConstraint)
        self.verticalLayout_2.setContentsMargins(-1, 0, -1, -1)
        self.verticalLayout_2.setObjectName(_fromUtf8("verticalLayout_2"))
        self.hzTop = QtGui.QHBoxLayout()
        self.hzTop.setObjectName(_fromUtf8("hzTop"))
        self.videoPlayer = phonon.Phonon.VideoPlayer(self.centralwidget)
        self.videoPlayer.setObjectName(_fromUtf8("videoPlayer"))
        self.hzTop.addWidget(self.videoPlayer)
        self.vertPlots = QtGui.QVBoxLayout()
        self.vertPlots.setObjectName(_fromUtf8("vertPlots"))
        self.hzTop.addLayout(self.vertPlots)
        self.gyroPlots = QtGui.QVBoxLayout()
        self.gyroPlots.setObjectName(_fromUtf8("gyroPlots"))
        self.hzTop.addLayout(self.gyroPlots)
        self.verticalLayout_2.addLayout(self.hzTop)
        self.gridLayout = QtGui.QGridLayout()
        self.gridLayout.setObjectName(_fromUtf8("gridLayout"))
        self.label_3 = QtGui.QLabel(self.centralwidget)
        self.label_3.setObjectName(_fromUtf8("label_3"))
        self.gridLayout.addWidget(self.label_3, 1, 1, 1, 1)
        self.comboIMU = QtGui.QComboBox(self.centralwidget)
        self.comboIMU.setObjectName(_fromUtf8("comboIMU"))
        self.gridLayout.addWidget(self.comboIMU, 1, 2, 1, 1)
        self.comboData = QtGui.QComboBox(self.centralwidget)
        self.comboData.setObjectName(_fromUtf8("comboData"))
        self.gridLayout.addWidget(self.comboData, 0, 2, 1, 1)
        self.label_2 = QtGui.QLabel(self.centralwidget)
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.gridLayout.addWidget(self.label_2, 0, 1, 1, 1)
        self.btnExit = QtGui.QPushButton(self.centralwidget)
        self.btnExit.setObjectName(_fromUtf8("btnExit"))
        self.gridLayout.addWidget(self.btnExit, 0, 0, 1, 1)
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.buttonPlay = QtGui.QPushButton(self.centralwidget)
        self.buttonPlay.setObjectName(_fromUtf8("buttonPlay"))
        self.verticalLayout.addWidget(self.buttonPlay)
        self.buttonPause = QtGui.QPushButton(self.centralwidget)
        self.buttonPause.setObjectName(_fromUtf8("buttonPause"))
        self.verticalLayout.addWidget(self.buttonPause)
        self.buttonStop = QtGui.QPushButton(self.centralwidget)
        self.buttonStop.setObjectName(_fromUtf8("buttonStop"))
        self.verticalLayout.addWidget(self.buttonStop)
        self.gridLayout.addLayout(self.verticalLayout, 1, 0, 1, 1)
        self.verticalLayout_2.addLayout(self.gridLayout)
        self.gridLayout_2.addLayout(self.verticalLayout_2, 0, 0, 1, 1)
        self.positionSlider = QtGui.QSlider(self.centralwidget)
        self.positionSlider.setOrientation(QtCore.Qt.Horizontal)
        self.positionSlider.setObjectName(_fromUtf8("positionSlider"))
        self.gridLayout_2.addWidget(self.positionSlider, 1, 0, 1, 1)
        IMU_Phonon.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(IMU_Phonon)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 1027, 22))
        self.menubar.setObjectName(_fromUtf8("menubar"))
        self.menuExit = QtGui.QMenu(self.menubar)
        self.menuExit.setObjectName(_fromUtf8("menuExit"))
        IMU_Phonon.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(IMU_Phonon)
        self.statusbar.setObjectName(_fromUtf8("statusbar"))
        IMU_Phonon.setStatusBar(self.statusbar)
        self.menubar.addAction(self.menuExit.menuAction())

        self.retranslateUi(IMU_Phonon)
        QtCore.QObject.connect(self.btnExit, QtCore.SIGNAL(_fromUtf8("clicked()")), IMU_Phonon.close)
        QtCore.QMetaObject.connectSlotsByName(IMU_Phonon)
        IMU_Phonon.setTabOrder(self.btnExit, self.comboData)
        IMU_Phonon.setTabOrder(self.comboData, self.comboIMU)
        IMU_Phonon.setTabOrder(self.comboIMU, self.buttonPlay)
        IMU_Phonon.setTabOrder(self.buttonPlay, self.buttonPause)
        IMU_Phonon.setTabOrder(self.buttonPause, self.buttonStop)
        IMU_Phonon.setTabOrder(self.buttonStop, self.positionSlider)

    def retranslateUi(self, IMU_Phonon):
        IMU_Phonon.setWindowTitle(QtGui.QApplication.translate("IMU_Phonon", "IMU Window", None, QtGui.QApplication.UnicodeUTF8))
        self.label_3.setText(QtGui.QApplication.translate("IMU_Phonon", "IMU", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("IMU_Phonon", "Data Set", None, QtGui.QApplication.UnicodeUTF8))
        self.btnExit.setText(QtGui.QApplication.translate("IMU_Phonon", "Exit", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonPlay.setText(QtGui.QApplication.translate("IMU_Phonon", "Play", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonPause.setText(QtGui.QApplication.translate("IMU_Phonon", "Pause", None, QtGui.QApplication.UnicodeUTF8))
        self.buttonStop.setText(QtGui.QApplication.translate("IMU_Phonon", "Stop", None, QtGui.QApplication.UnicodeUTF8))
        self.menuExit.setTitle(QtGui.QApplication.translate("IMU_Phonon", "Exit", None, QtGui.QApplication.UnicodeUTF8))

from PyQt4 import phonon
