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

#define DOIP_CLIENT_CONFIG_PATH "/config/doip.ini"

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
        sourceAddress = settings.value("client/sourceAddress").toString().toUShort(nullptr, 16);
        targetAddress = settings.value("client/targetAddress").toString().toUShort(nullptr, 16);
        activeType = settings.value("client/activeType").toString().toInt(nullptr, 16);
        requireRoutingActivation = settings.value("client/requireRoutingActivation").toBool();
        aliveCheckResponse = settings.value("client/aliveCheckResponse").toBool();
        useOEMSpecific = settings.value("client/useOEMSpecific").toBool();
        additionalOEMSpecific = QByteArray::fromHex(settings.value("client/additionalOEMSpecific").toString().toUtf8());
        futureStandardization = QByteArray::fromHex(settings.value("client/futureStandardization").toString().toUtf8());
        routingActivationWaitTime = settings.value("client/routingActivationWaitTime").toInt();
        controlTime = settings.value("client/controlTime").toInt();
    }

    void save() {
        settings.setValue("client/version", QString("%1").arg(version, 2 /*width*/, 16 /*base*/, QChar('0') /*fill*/));
        settings.setValue("client/sourceAddress", QString("%1").arg(sourceAddress, 4 /*width*/, 16 /*base*/, QChar('0') /*fill*/));
        settings.setValue("client/targetAddress", QString("%1").arg(targetAddress, 4 /*width*/, 16 /*base*/, QChar('0') /*fill*/));
        settings.setValue("client/activeType", QString("%1").arg(activeType, 2 /*width*/, 16 /*base*/, QChar('0') /*fill*/));
        settings.setValue("client/requireRoutingActivation", requireRoutingActivation);
        settings.setValue("client/aliveCheckResponse", aliveCheckResponse);
        settings.setValue("client/useOEMSpecific", useOEMSpecific);
        settings.setValue("client/additionalOEMSpecific", QString(additionalOEMSpecific.toHex(' ')));
        settings.setValue("client/futureStandardization", QString(futureStandardization.toHex(' ')));
        settings.setValue("client/routingActivationWaitTime", routingActivationWaitTime);
        settings.setValue("client/controlTime", controlTime);
    }

    int getVersion() const { return version; }
    void setVersion(int value) { version = value; }

    unsigned short getSourceAddress() const { return sourceAddress; }
    void setSourceAddress(unsigned short value) {sourceAddress = value; }

    unsigned short getTargetAddress() const { return targetAddress; }
    void setTargetAddress(unsigned short value) {targetAddress = value; }

    int getActiveType() const { return activeType; }
    void setActiveType(int value) { activeType = value; }

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

    int getRoutingActivationWaitTime() const { return routingActivationWaitTime; }
    void setRoutingActivationWaitTime(int value) { routingActivationWaitTime = value; }

    int getControlTime() const { return controlTime; }
    void setControlTime(int value) { controlTime = value; }

private:
    DoIPClientConfig(): settings(QCoreApplication::applicationDirPath()+QString(DOIP_CLIENT_CONFIG_PATH), QSettings::IniFormat) {
        load();
    }

    ~DoIPClientConfig() {
        save();
    }

    QSettings settings;

    int version { 2 };
    unsigned short sourceAddress { 0x0e00 };
    unsigned short targetAddress { 0xbbbb };
    int activeType { 0 };
    bool requireRoutingActivation { true };
    bool aliveCheckResponse { true };
    bool useOEMSpecific { false };
    QByteArray additionalOEMSpecific {4, 0};;
    QByteArray futureStandardization {4, 0};;
    int routingActivationWaitTime { 2000 };
    int controlTime { 2000 };
};

}// namespace figkey

#endif // !DOIP_CLIENT_CONFIG_H
