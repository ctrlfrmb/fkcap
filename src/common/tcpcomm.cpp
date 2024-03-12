/**
 * @file    tcpcomm.h
 * @ingroup opensource::ctrlfrmb
 * @brief
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) opensource::ctrlfrmb 2024-2034
 */
#include "common/tcpcomm.h"

TCPComm::TCPComm(const QString &localIp, const QString &serverIp,
                 int serverPort, bool isServer, QObject *parent)
    : BaseComm(localIp, serverIp, serverPort, isServer, parent)
{}

TCPComm::~TCPComm() {
    stop();
}

bool TCPComm::start() {
    if (m_isRunning) {
        return false;
    }

    bool result = false;
    if (m_isServer) {
        result = m_server.listen(QHostAddress(m_localIp), m_serverPort);
        if (result) {
            connect(&m_server, &QTcpServer::newConnection,
                    this, &TCPComm::newConnection);
        }
    } else {
        m_socket = new QTcpSocket(this);
        if (!m_socket->bind(QHostAddress(m_localIp))) {
            qDebug() << "Failed to bind to local IP";
            return false;
        }
        m_socket->connectToHost(m_serverIp, m_serverPort);
        connect(m_socket, &QTcpSocket::readyRead, this, &TCPComm::readyRead);
        result = m_socket->isOpen();
        if (!result) {  // 连接失败
            m_socket->deleteLater();
            m_socket = nullptr;
        }
    }
    m_isRunning = result;
    return result;
}

qint64 TCPComm::sendData(const QByteArray &/*data*/) {
    return 0;
}

void TCPComm::stop() {
    if (!m_isRunning)
        return;

    if (m_socket) {
        m_socket->disconnectFromHost();
        delete m_socket;
        m_socket = nullptr;
    }

    if (m_server.isListening()) {
        m_server.close();
    }

    m_isRunning = false;
}

void TCPComm::newConnection() {
    if (m_socket) {
        m_socket->disconnectFromHost();
        delete m_socket;
    }

    m_socket = m_server.nextPendingConnection();
    connect(m_socket, &QTcpSocket::readyRead,
            this, &TCPComm::readyRead);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &TCPComm::disconnected);
}

void TCPComm::readyRead() {
    QByteArray data = m_socket->readAll();
    emit dataReceived(data);
}

void TCPComm::disconnected() {
    m_socket->deleteLater();
    m_socket = nullptr;
}
