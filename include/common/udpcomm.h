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
    UDPComm(const QString& localIp, const QString& serverIp,
            int serverPort, bool isServer, QObject *parent = nullptr);
    virtual ~UDPComm();

    virtual bool start() override;
    virtual qint64 sendData(const QByteArray &data) override;
    virtual void stop() override;

private slots:
    void readyRead();

private:
    QUdpSocket m_socket;
};


#endif // !QT_UDP_COMMON_H
