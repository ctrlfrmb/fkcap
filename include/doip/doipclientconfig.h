/**
 * @file    doipclientconfig.h
 * @ingroup figkey
 * @brief   
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) figkey 2024-2034
 */

#pragma once

#ifndef DOIP_CLIENT_CONFIG_H
#define DOIP_CLIENT_CONFIG_H

#include <QSettings>
#include <QString>
#include <QByteArray>
#include <QApplication>

#define DOIP_CLIENT_PATH "/config/doipclient.ini"

namespace figkey {

class DoIPClientConfig {
public:
    // Delete copy constructor and assignment operator to ensure uniqueness of the doip client config
    DoIPClientConfig(const DoIPClientConfig&) = delete;
    DoIPClientConfig(DoIPClientConfig&&) = delete;
    DoIPClientConfig& operator=(const DoIPClientConfig&) = delete;
    DoIPClientConfig& operator=(DoIPClientConfig&&) = delete;

    // Retrieve an instance of the doip client config(singleton pattern)
    static DoIPClientConfig& Instance() {
        static DoIPClientConfig obj;
        return obj;
    }

    int getVersion() const { return settings.value("client/version").toString().toInt(nullptr, 16); }
    void setVersion(int value) { settings.setValue("client/version",
                                                   QString("%1").arg(value, 2 /*width*/, 16 /*base*/, QChar('0') /*fill*/)); }

    std::string getLocalIP() const { return settings.value("client/localIP").toString().toStdString(); }
    void setLocalIP(const QString& value) { settings.setValue("client/localIP", value); }

    std::string getServerIP() const { return settings.value("client/serverIP").toString().toStdString(); }
    void setServerIP(const QString& value) { settings.setValue("client/serverIP", value); }

    int getUdpPort() const { return settings.value("client/udpPort").toInt(); }
    void setUdpPort(int value) { settings.setValue("client/udpPort", value); }

    int getTcpPort() const { return settings.value("client/tcpPort").toInt(); }
    void setTcpPort(int value) { settings.setValue("client/tcpPort", value); }

    // 对于例如sourceAddress这样的16进制数，我们按照字符串保存和读取，然后进行相应的转换
    unsigned short getSourceAddress() const { return settings.value("client/sourceAddress").toString().toUShort(nullptr, 16); }
    void setSourceAddress(unsigned short value) {
        settings.setValue("client/sourceAddress", QString("%1").arg(value, 4 /*width*/, 16 /*base*/, QChar('0') /*fill*/));
    }

    unsigned short getTargetAddress() const { return settings.value("client/targetAddress").toString().toUShort(nullptr, 16); }
    void setTargetAddress(unsigned short value) {
        settings.setValue("client/targetAddress", QString("%1").arg(value, 4 /*width*/, 16 /*base*/, QChar('0') /*fill*/));
    }

    int getActiveType() const { return settings.value("client/activeType").toString().toInt(nullptr, 16); }
    void setActiveType(int value) { settings.setValue("client/activeType",
                                                      QString("%1").arg(value, 2 /*width*/, 16 /*base*/, QChar('0') /*fill*/)); }

    bool getAutoVehicleDiscovery() const { return settings.value("client/autoVehicleDiscovery").toBool(); }
    void setAutoVehicleDiscovery(bool value) { settings.setValue("client/autoVehicleDiscovery", value); }

    bool getUseOEMSpecific() const { return settings.value("client/useOEMSpecific").toBool(); }
    void setUseOEMSpecific(bool value) { settings.setValue("client/useOEMSpecific", value); }

    QByteArray getAdditionalOEMSpecific() const {
        return QByteArray::fromHex(settings.value("client/additionalOEMSpecific").toString().toUtf8());
    }
    void setAdditionalOEMSpecific(const QByteArray& value) {
        QString hexValue = value.toHex();
        QString hexValueWithSpaces;

        for (int i = 0; i < hexValue.size(); i+=2) {
            hexValueWithSpaces.append(hexValue.mid(i, 2));
            hexValueWithSpaces.append(' ');
        }

        settings.setValue("client/additionalOEMSpecific", hexValueWithSpaces.trimmed());
    }

    QByteArray getFutureStandardization() const {
        return QByteArray::fromHex(settings.value("client/futureStandardization").toString().toUtf8());
    }
    void setFutureStandardization(const QByteArray& value) { 
        QString hexValue = value.toHex();
        QString hexValueWithSpaces;

        for (int i = 0; i < hexValue.size(); i+=2) {
            hexValueWithSpaces.append(hexValue.mid(i, 2));
            hexValueWithSpaces.append(' ');
        }

        settings.setValue("client/futureStandardization", hexValueWithSpaces.trimmed());
    }

    int getConnTimeout() const { return settings.value("client/connTimeout").toInt(); }
    void setConnTimeout(int value) { settings.setValue("client/connTimeout", value); }

    int getCtrlTime() const { return settings.value("client/ctrlTime").toInt(); }
    void setCtrlTime(int value) { settings.setValue("client/ctrlTime", value); }

    int getDiagnosticMessageTime() const { return settings.value("client/diagnosticMessageTime").toInt(); }
    void setDiagnosticMessageTime(int value) { settings.setValue("client/diagnosticMessageTime", value); }

    int getVehicleDiscoveryTime() const { return settings.value("client/vehicleDiscoveryTime").toInt(); }
    void setVehicleDiscoveryTime(int value) { settings.setValue("client/vehicleDiscoveryTime", value); }

    int getTcpInitialInactivityTime() const { return settings.value("client/tcpInitialInactivityTime").toInt(); }
    void setTcpInitialInactivityTime(int value) { settings.setValue("client/tcpInitialInactivityTime", value); }

private:
    // DoIP Client config constructor
    DoIPClientConfig(): settings(QCoreApplication::applicationDirPath()+QString(DOIP_CLIENT_PATH), QSettings::IniFormat) {

    }

    // DoIP Client config destructor
    ~DoIPClientConfig() {}

    QSettings settings;
};

}// namespace figkey

#endif // !DOIP_CLIENT_CONFIG_H
