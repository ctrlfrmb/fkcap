/**
 * @file    udpcomm.h
 * @ingroup opensource::ctrlfrmb
 * @brief
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) opensource::ctrlfrmb 2024-2034
 */
#include "common/udpcomm.h"

UDPComm::UDPComm(const QString& localIp, const QString& serverIp,
                 int serverPort, bool isServer, QObject *parent)
     : BaseComm(localIp, serverIp, serverPort, isServer, parent), m_socket(this)
{
}

UDPComm::~UDPComm() {
    stop();
}

// 作为客户端，绑定本地IP，端口随机；作为服务器，绑定本地IP和特定端口。
bool UDPComm::start() {
    bool result = false;
    if (m_isServer) {
        result = m_socket.bind(QHostAddress(m_localIp), m_serverPort);
    } else {
        result = m_socket.bind(QHostAddress(m_localIp));
    }

    if (result) {
        connect(&m_socket, &QUdpSocket::readyRead, this, &UDPComm::readyRead);
    }
    return result;
}

qint64 UDPComm::sendData(const QByteArray &data) {
    if (!m_socket.isOpen()) {
        return -1;
    }

    return m_socket.writeDatagram(data, QHostAddress(m_serverIp), m_serverPort);
}

void UDPComm::stop() {
    if (m_socket.isOpen())
        m_socket.close();
}

void UDPComm::readyRead() {
    while (m_socket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket.pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_socket.readDatagram(datagram.data(), datagram.size(),
                            &sender, &senderPort);

        if (m_isServer || (sender == QHostAddress(m_serverIp) && senderPort == m_serverPort)) {
            emit dataReceived(datagram);
        }
    }
}
