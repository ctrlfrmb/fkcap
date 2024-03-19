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

#define DOIP_CLIENT_PATH "/config/doip.ini"

namespace figkey {

class DoIPClientConfig {
public:
    DoIPClientConfig(const DoIPClientConfig&) = delete;
    DoIPClientConfig(DoIPClientConfig&&) = delete;
    DoIPClientConfig& operator=(const DoIPClientConfig&) = delete;
    DoIPClientConfig& operator=(DoIPClientConfig&&) = delete;

    static DoIPClientConfig& Instance() {
        static DoIPClientConfig obj;
        return obj;
    }

    void load() {
        version = settings.value("client/version").toString().toInt(nullptr, 16);
        localIP = settings.value("client/localIP").toString().toStdString();
        serverIP = settings.value("client/serverIP").toString().toStdString();
        udpPort = settings.value("client/udpPort").toInt();
        tcpPort =  settings.value("client/tcpPort").toInt();
        sourceAddress = settings.value("client/sourceAddress").toString().toUShort(nullptr, 16);
        targetAddress = settings.value("client/targetAddress").toString().toUShort(nullptr, 16);
        activeType = settings.value("client/activeType").toString().toInt(nullptr, 16);
        autoVehicleDiscovery = settings.value("client/autoVehicleDiscovery").toBool();
        requireRoutingActivation = settings.value("client/requireRoutingActivation").toBool();
        aliveCheckResponse = settings.value("client/aliveCheckResponse").toBool();
        useOEMSpecific = settings.value("client/useOEMSpecific").toBool();
        additionalOEMSpecific = QByteArray::fromHex(settings.value("client/additionalOEMSpecific").toString().toUtf8());
        futureStandardization = QByteArray::fromHex(settings.value("client/futureStandardization").toString().toUtf8());
        connTimeout = settings.value("client/connTimeout").toInt();
        ctrlTime = settings.value("client/ctrlTime").toInt();
        diagnosticMessageTime = settings.value("client/diagnosticMessageTime").toInt();
        vehicleDiscoveryTime = settings.value("client/vehicleDiscoveryTime").toInt();
        routingActivationWaitTime = settings.value("client/routingActivationWaitTime").toInt();
    }

    void save() {
        settings.setValue("client/version", QString("%1").arg(version, 2 /*width*/, 16 /*base*/, QChar('0') /*fill*/));
        settings.setValue("client/localIP", QString::fromStdString(localIP));
        settings.setValue("client/serverIP", QString::fromStdString(serverIP));
        settings.setValue("client/udpPort", udpPort);
        settings.setValue("client/tcpPort", tcpPort);
        settings.setValue("client/sourceAddress", QString("%1").arg(sourceAddress, 4 /*width*/, 16 /*base*/, QChar('0') /*fill*/));
        settings.setValue("client/targetAddress", QString("%1").arg(targetAddress, 4 /*width*/, 16 /*base*/, QChar('0') /*fill*/));
        settings.setValue("client/activeType", QString("%1").arg(activeType, 2 /*width*/, 16 /*base*/, QChar('0') /*fill*/));
        settings.setValue("client/autoVehicleDiscovery", autoVehicleDiscovery);
        settings.setValue("client/requireRoutingActivation", requireRoutingActivation);
        settings.setValue("client/aliveCheckResponse", aliveCheckResponse);
        settings.setValue("client/useOEMSpecific", useOEMSpecific);
        settings.setValue("client/additionalOEMSpecific", QString(additionalOEMSpecific.toHex(' ')));
        settings.setValue("client/futureStandardization", QString(futureStandardization.toHex(' ')));
        settings.setValue("client/connTimeout", connTimeout);
        settings.setValue("client/ctrlTime", ctrlTime);
        settings.setValue("client/diagnosticMessageTime", diagnosticMessageTime);
        settings.setValue("client/vehicleDiscoveryTime", vehicleDiscoveryTime);
        settings.setValue("client/routingActivationWaitTime", routingActivationWaitTime);
    }

    int getVersion() const { return version; }
    void setVersion(int value) { version = value; }

    std::string getLocalIP() const { return localIP; }
    void setLocalIP(const std::string& value) { localIP = value; }

    std::string getServerIP() const { return serverIP; }
    void setServerIP(const std::string& value) { serverIP = value; }

    int getUdpPort() const { return udpPort; }
    void setUdpPort(int value) { udpPort = value; }

    int getTcpPort() const { return tcpPort; }
    void setTcpPort(int value) { tcpPort = value; }

    unsigned short getSourceAddress() const { return sourceAddress; }
    void setSourceAddress(unsigned short value) {sourceAddress = value; }

    unsigned short getTargetAddress() const { return targetAddress; }
    void setTargetAddress(unsigned short value) {targetAddress = value; }

    int getActiveType() const { return activeType; }
    void setActiveType(int value) { activeType = value; }

    bool getAutoVehicleDiscovery() const { return autoVehicleDiscovery; }
    void setAutoVehicleDiscovery(bool value) { autoVehicleDiscovery = value; }

    bool getRequireRoutingActivation() const { return requireRoutingActivation; }
    void setRequireRoutingActivation(bool value) { requireRoutingActivation = value;}

    bool getAliveCheckResponse() const { return aliveCheckResponse; }
    void setAliveCheckResponse(bool value) { aliveCheckResponse = value; }

    bool getUseOEMSpecific() const { return useOEMSpecific; }
    void setUseOEMSpecific(bool value) { useOEMSpecific = value; }

    QByteArray getAdditionalOEMSpecific() const {return additionalOEMSpecific; }
    void setAdditionalOEMSpecific(const QByteArray& value) { additionalOEMSpecific = value; }

    QByteArray getFutureStandardization() const { return futureStandardization; }
    void setFutureStandardization(const QByteArray& value) { futureStandardization = value; }

    int getConnTimeout() const { return connTimeout; }
    void setConnTimeout(int value) { connTimeout = value; }

    int getCtrlTime() const { return ctrlTime; }
    void setCtrlTime(int value) { ctrlTime = value; }

    int getDiagnosticMessageTime() const { return diagnosticMessageTime;  }
    void setDiagnosticMessageTime(int value) { diagnosticMessageTime = value; }

    int getVehicleDiscoveryTime() const { return vehicleDiscoveryTime; }
    void setVehicleDiscoveryTime(int value) { vehicleDiscoveryTime = value; }

    int getRoutingActivationWaitTime() const { return routingActivationWaitTime; }
    void setRoutingActivationWaitTime(int value) { routingActivationWaitTime = value; }

private:
    DoIPClientConfig(): settings(QCoreApplication::applicationDirPath()+QString(DOIP_CLIENT_PATH), QSettings::IniFormat) {
        load();
    }

    ~DoIPClientConfig() {
        save();
    }

    QSettings settings;

    int version;
    std::string localIP;
    std::string serverIP;
    int udpPort;
    int tcpPort;
    unsigned short sourceAddress;
    unsigned short targetAddress;
    int activeType;
    bool autoVehicleDiscovery;
    bool requireRoutingActivation;
    bool aliveCheckResponse;
    bool useOEMSpecific;
    QByteArray additionalOEMSpecific;
    QByteArray futureStandardization;
    int connTimeout;
    int ctrlTime;
    int diagnosticMessageTime;
    int vehicleDiscoveryTime;
    int routingActivationWaitTime;
};

}// namespace figkey

#endif // !DOIP_CLIENT_CONFIG_H
