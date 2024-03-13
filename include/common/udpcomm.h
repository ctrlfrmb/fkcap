/**
 * @file    udpcomm.h
 * @ingroup opensource::ctrlfrmb
 * @brief   
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) opensource::ctrlfrmb 2024-2034
 */

#pragma once

#ifndef QT_UDP_COMMON_H
#define QT_UDP_COMMON_H

#include "basecomm.h"
#include <QUdpSocket>

class UDPComm : public BaseComm {
public:
    UDPComm(const QString& clientIp, const QString& serverIp,
            int serverPort, bool isServer, QObject *parent = nullptr);
    virtual ~UDPComm();

    virtual bool start() override;
    virtual bool sendData(const QByteArray &data) override;
    virtual void stop() override;

private slots:
    void readyRead();

private:
    QUdpSocket m_socket;
    bool m_isRunning = false;
    QHostAddress m_partAddress;
    quint16 m_partPort;
};


#endif // !QT_UDP_COMMON_H
