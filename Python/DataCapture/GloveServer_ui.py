# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'GloveServer.ui'
#
# Created: Wed Apr 27 02:19:06 2011
#      by: PyQt4 UI code generator 4.7.7
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_GloveServer(object):
    def setupUi(self, GloveServer):
        GloveServer.setObjectName(_fromUtf8("GloveServer"))
        GloveServer.resize(567, 457)
        self.centralwidget = QtGui.QWidget(GloveServer)
        self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
        self.label = QtGui.QLabel(self.centralwidget)
        self.label.setGeometry(QtCore.QRect(50, 30, 62, 16))
        self.label.setObjectName(_fromUtf8("label"))
        self.editRate = QtGui.QLineEdit(self.centralwidget)
        self.editRate.setGeometry(QtCore.QRect(180, 30, 113, 22))
        self.editRate.setObjectName(_fromUtf8("editRate"))
        self.label_2 = QtGui.QLabel(self.centralwidget)
        self.label_2.setGeometry(QtCore.QRect(50, 90, 101, 16))
        self.label_2.setObjectName(_fromUtf8("label_2"))
        self.editPackets = QtGui.QLineEdit(self.centralwidget)
        self.editPackets.setGeometry(QtCore.QRect(180, 90, 113, 22))
        self.editPackets.setObjectName(_fromUtf8("editPackets"))
        self.btnStartStop = QtGui.QPushButton(self.centralwidget)
        self.btnStartStop.setGeometry(QtCore.QRect(50, 150, 114, 32))
        self.btnStartStop.setObjectName(_fromUtf8("btnStartStop"))
        self.btnQuit = QtGui.QPushButton(self.centralwidget)
        self.btnQuit.setGeometry(QtCore.QRect(190, 150, 114, 32))
        self.btnQuit.setObjectName(_fromUtf8("btnQuit"))
        GloveServer.setCentralWidget(self.centralwidget)
        self.menubar = QtGui.QMenuBar(GloveServer)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 567, 22))
        self.menubar.setObjectName(_fromUtf8("menubar"))
        GloveServer.setMenuBar(self.menubar)
        self.statusbar = QtGui.QStatusBar(GloveServer)
        self.statusbar.setObjectName(_fromUtf8("statusbar"))
        GloveServer.setStatusBar(self.statusbar)

        self.retranslateUi(GloveServer)
        QtCore.QObject.connect(self.btnQuit, QtCore.SIGNAL(_fromUtf8("clicked()")), GloveServer.close)
        QtCore.QMetaObject.connectSlotsByName(GloveServer)

    def retranslateUi(self, GloveServer):
        GloveServer.setWindowTitle(QtGui.QApplication.translate("GloveServer", "MainWindow", None, QtGui.QApplication.UnicodeUTF8))
        self.label.setText(QtGui.QApplication.translate("GloveServer", "Rate", None, QtGui.QApplication.UnicodeUTF8))
        self.label_2.setText(QtGui.QApplication.translate("GloveServer", "Packets Captured", None, QtGui.QApplication.UnicodeUTF8))
        self.btnStartStop.setText(QtGui.QApplication.translate("GloveServer", "Start", None, QtGui.QApplication.UnicodeUTF8))
        self.btnQuit.setText(QtGui.QApplication.translate("GloveServer", "Quit", None, QtGui.QApplication.UnicodeUTF8))

