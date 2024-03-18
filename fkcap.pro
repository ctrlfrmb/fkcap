#-------------------------------------------------
#
# Project created by QtCreator 2024-02-26T13:31:14
#
#-------------------------------------------------

QT       += core gui sql network
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = fkcap
TEMPLATE = app

INCLUDEPATH += $$PWD\include
INCLUDEPATH += $$PWD\ui
INCLUDEPATH += $$PWD\ipcap\include
INCLUDEPATH += $$PWD\include\npcap1.13\include

SOURCES += main.cpp\
    src/common/basecomm.cpp \
    src/common/tcpcomm.cpp \
    src/common/udpcomm.cpp \
    src/common/tcpclient.cpp \
    src/common/udpclient.cpp \
    ipcap/src/protocol/doip.cpp \
    ipcap/src/protocol/ip.cpp \
    ipcap/src/protocol/uds.cpp \
    ipcap/src/config.cpp \
    ipcap/src/ipcap.cpp \
    ipcap/src/packet.cpp \
    src/sqlite.cpp \
    src/packeinfo.cpp \
    src/doip/doipclient.cpp \
    src/doip/diagnosticmessagehandler.cpp \
    src/doip/doipgenericheaderhandler.cpp \
    ui/doipsettingwindow.cpp \
    ui/mainwindow.cpp \
    ui/devicewindow.cpp \
    ui/filterwindow.cpp \
    ui/networkassistwindow.cpp \
    ui/networkhelper.cpp

HEADERS  += include/common/basecomm.h \
    include/common/tcpcomm.h \
    include/common/udpcomm.h \
    include/common/thread_pool.hpp \
    include/common/tcpclient.h \
    include/common/udpclient.h \
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
    include/sqlite.h \
    include/packeinfo.h \
    include/doip/doipclientconfig.h \
    include/doip/doipclient.h \
    include/doip/diagnosticmessagehandler.h \
    include/doip/doipgenericheaderhandler.h \
    ui/doipsettingwindow.h \
    ui/mainwindow.h \
    ui/devicewindow.h \
    ui/filterwindow.h \
    ui/networkassistwindow.h \
    ui/networkhelper.h

FORMS    += ui/mainwindow.ui \
    ui/devicewindow.ui \
    ui/doipsettingwindow.ui \
    ui/filterwindow.ui \
    ui/doipclientwindow.ui \
    ui/networkassistwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/include/npcap1.13/lib/x86_64/ -lwpcap
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/include/npcap1.13/lib/x86_64/ -lwpcap
else:unix: LIBS += -L$$PWD/include/npcap1.13/lib/x86_64/ -lwpcap

LIBS += -lws2_32

INCLUDEPATH += $$PWD/include/npcap1.13/lib/x86_64
DEPENDPATH += $$PWD/include/npcap1.13/lib/x86_64

RESOURCES += \
    resource.qrc

# 使用你的图标文件替换“icon.ico”，如果你的目录结构不同，适当地改变路径。
RC_ICONS = resource/icons/main_window.ico

