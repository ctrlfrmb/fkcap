#-------------------------------------------------
#
# Project created by QtCreator 2024-02-26T13:31:14
#
#-------------------------------------------------

QT       += core gui sql
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fkcap
TEMPLATE = app

INCLUDEPATH += $$PWD\include
INCLUDEPATH += $$PWD\ui
INCLUDEPATH += $$PWD\ipcap\include
INCLUDEPATH += $$PWD\include\npcap1.13\include

SOURCES += main.cpp\
        ui/mainwindow.cpp \
    ipcap/src/protocol/doip.cpp \
    ipcap/src/protocol/ip.cpp \
    ipcap/src/protocol/uds.cpp \
    ipcap/src/config.cpp \
    ipcap/src/ipcap.cpp \
    ipcap/src/packet.cpp \
    ui/devicewindow.cpp \
    src/sqlite.cpp \
    src/packeinfo.cpp \
    ui/filterwindow.cpp \
    ui/doipclientwindow.cpp \
    src/doip/doipclient.cpp \
    src/doip/diagnosticmessagehandler.cpp \
    src/doip/doipgenericheaderhandler.cpp \
    src/common/tcpclient.cpp \
    src/common/udpclient.cpp

HEADERS  += ui/mainwindow.h \
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
    ui/devicewindow.h \
    include/sqlite.h \
    include/packeinfo.h \
    ui/filterwindow.h \
    ui/doipclientwindow.h \
    include/doip/doipclientconfig.h \
    include/doip/doipclient.h \
    include/doip/diagnosticmessagehandler.h \
    include/doip/doipgenericheaderhandler.h \
    include/common/tcpclient.h \
    include/common/udpclient.h

FORMS    += ui/mainwindow.ui \
    ui/devicewindow.ui \
    ui/filterwindow.ui \
    ui/doipclientwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/include/npcap1.13/lib/x86_64/ -lwpcap
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/include/npcap1.13/lib/x86_64/ -lwpcap
else:unix: LIBS += -L$$PWD/include/npcap1.13/lib/x86_64/ -lwpcap

INCLUDEPATH += $$PWD/include/npcap1.13/lib/x86_64
DEPENDPATH += $$PWD/include/npcap1.13/lib/x86_64

RESOURCES += \
    resource.qrc
