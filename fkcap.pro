#-------------------------------------------------
#
# Project created by QtCreator 2024-02-26T13:31:14
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fkcap
TEMPLATE = app

INCLUDEPATH += $$PWD\include
INCLUDEPATH += $$PWD\ipcap\include
INCLUDEPATH += $$PWD\include\npcap1.13\include

SOURCES += main.cpp\
        mainwindow.cpp \
    ipcap/src/protocol/doip.cpp \
    ipcap/src/protocol/ip.cpp \
    ipcap/src/protocol/uds.cpp \
    ipcap/src/config.cpp \
    ipcap/src/ipcap.cpp \
    ipcap/src/packet.cpp \
    devicewindow.cpp

HEADERS  += mainwindow.h \
    include/common/thread_pool.hpp \
    include/npcap1.13/include/pcap/bluetooth.h \
    include/npcap1.13/include/pcap/bpf.h \
    include/npcap1.13/include/pcap/can_socketcan.h \
    include/npcap1.13/include/pcap/compiler-tests.h \
    include/npcap1.13/include/pcap/dlt.h \
    include/npcap1.13/include/pcap/funcattrs.h \
    include/npcap1.13/include/pcap/ipnet.h \
    include/npcap1.13/include/pcap/namedb.h \
    include/npcap1.13/include/pcap/nflog.h \
    include/npcap1.13/include/pcap/pcap-inttypes.h \
    include/npcap1.13/include/pcap/pcap.h \
    include/npcap1.13/include/pcap/sll.h \
    include/npcap1.13/include/pcap/socket.h \
    include/npcap1.13/include/pcap/usb.h \
    include/npcap1.13/include/pcap/vlan.h \
    include/npcap1.13/include/Packet32.h \
    include/npcap1.13/include/pcap-bpf.h \
    include/npcap1.13/include/pcap-namedb.h \
    include/npcap1.13/include/pcap.h \
    ipcap/include/protocol/doip.h \
    ipcap/include/protocol/ip.h \
    ipcap/include/protocol/uds.h \
    ipcap/include/config.h \
    ipcap/include/def.h \
    ipcap/include/ipcap.h \
    ipcap/include/packet.h \
    ipcap/include/testip.h \
    devicewindow.h

FORMS    += mainwindow.ui \
    devicewindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/include/npcap1.13/lib/x86_64/ -lwpcap
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/include/npcap1.13/lib/x86_64/ -lwpcap
else:unix: LIBS += -L$$PWD/include/npcap1.13/lib/x86_64/ -lwpcap

INCLUDEPATH += $$PWD/include/npcap1.13/lib/x86_64
DEPENDPATH += $$PWD/include/npcap1.13/lib/x86_64
