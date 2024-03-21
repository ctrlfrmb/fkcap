/**
 * @file    doipserverconfig.h
 * @ingroup figkey
 * @brief   
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) figkey 2024-2034
 */

#pragma once

#ifndef DOIP_SERVER_CONFIG_H
#define DOIP_SERVER_CONFIG_H

#include <QSettings>
#include <QString>
#include <QByteArray>
#include <QApplication>

#define DOIP_SERVER_CONFIG_PATH "/config/doip.ini"

namespace figkey {

class DoIPServerConfig {
public:
    DoIPServerConfig(const DoIPServerConfig&) = delete;
    DoIPServerConfig(DoIPServerConfig&&) = delete;
    DoIPServerConfig& operator=(const DoIPServerConfig&) = delete;
    DoIPServerConfig& operator=(DoIPServerConfig&&) = delete;

    static DoIPServerConfig& Instance() {
        static DoIPServerConfig obj;
        return obj;
    }

    void load() {
        IpAddress = settings.value("server/IpAddress").toString();
        TcpPort = settings.value("server/TcpPort").toString().toUShort();
        Eid = QByteArray::fromHex(settings.value("server/Eid").toString().toUtf8());
        Gid = QByteArray::fromHex(settings.value("server/Gid").toString().toUtf8());
        Vin = QByteArray::fromHex(settings.value("server/Vin").toString().toUtf8());
    }

    void save() {
        settings.setValue("server/IpAddress", IpAddress);
        settings.setValue("server/TcpPort", QString::number(TcpPort));
        settings.setValue("server/Eid", QString(Eid.toHex(' ')));
        settings.setValue("server/Gid", QString(Gid.toHex(' ')));
        settings.setValue("server/Vin", QString(Vin.toHex(' ')));
    }

    QString getIpAddress() const {return IpAddress; }
    void setIpAddress(const QString& value) { IpAddress = value; }

    unsigned short getTcpPort() const {return TcpPort; }
    void setTcpPort(const unsigned short& value) { TcpPort = value; }

    QByteArray getEid() const {return Eid; }
    void setEid(const QByteArray& value) { Eid = value; }

    QByteArray getGid() const { return Gid; }
    void setGid(const QByteArray& value) { Gid = value; }

    QByteArray getVin() const { return Vin; }
    void setVin(const QByteArray& value) { Vin = value; }

private:
    DoIPServerConfig(): settings(QCoreApplication::applicationDirPath()+QString(DOIP_SERVER_CONFIG_PATH), QSettings::IniFormat) {
        load();
    }

    ~DoIPServerConfig() {
        save();
    }

    QSettings settings;

    QString IpAddress {"192.168.1.1"};
    unsigned short TcpPort { 13400 };
    QByteArray Eid {6, 0};
    QByteArray Gid {6, 0};
    QByteArray Vin {17, 0};
};

}// namespace figkey

#endif // !DOIP_SERVER_CONFIG_H
