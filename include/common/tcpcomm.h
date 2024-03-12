/**
 * @file    tcpcomm.h
 * @ingroup opensource::ctrlfrmb
 * @brief   
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) opensource::ctrlfrmb 2024-2034
 */

#pragma once

#ifndef QT_TCP_COMMON_H
#define QT_TCP_COMMON_H

#include "basecomm.h"
#include <QTcpServer>
#include <QTcpSocket>

class TCPComm : public BaseComm {
public:
    TCPComm(const QString& localIp, const QString& serverIp,
            int serverPort, bool isServer, QObject *parent = nullptr);
    virtual ~TCPComm();

    virtual bool start() override;
    virtual qint64 sendData(const QByteArray &data) override;
    virtual void stop() override;

private slots:
    void newConnection();
    void readyRead();
    void disconnected();

private:
    QTcpServer m_server;
    QTcpSocket *m_socket = nullptr;
    bool m_isRunning = false;
};

#endif // !QT_TCP_COMMON_H
