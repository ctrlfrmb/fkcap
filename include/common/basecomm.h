/**
 * @file    basecomm.h
 * @ingroup opensource::ctrlfrmb
 * @brief   
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) opensource::ctrlfrmb 2024-2034
 */

#pragma once

#ifndef QT_BASE_COMMON_H
#define QT_BASE_COMMON_H
#pragma once

#include <QObject>
#include <QString>

class BaseComm : public QObject {
    Q_OBJECT

public:
    BaseComm(const QString& clientIp, const QString& serverIp,
             int serverPort, bool isServer, QObject *parent = nullptr);

    virtual bool start() = 0;
    virtual bool sendData(const QByteArray &data) = 0;
    virtual void stop() = 0;

    QString getLastError() const;

signals:
    void dataReceived(const QByteArray& data);

protected:
    QString m_clientIp;
    QString m_serverIp;
    int m_serverPort;
    bool m_isServer;
    QString m_lastError;
};

#endif // !QT_BASE_COMMON_H
