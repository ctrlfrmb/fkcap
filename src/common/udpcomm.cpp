#include "common/udpcomm.h"

UDPComm::UDPComm(const QString& clientIp, const QString& serverIp,
                 int serverPort, bool isServer, QObject *parent)
     : BaseComm(clientIp, serverIp, serverPort, isServer, parent), m_socket(this)
{
}

UDPComm::~UDPComm() {
    stop();
}

bool UDPComm::start() {
    if (m_isRunning) {
        return true;
    }

    if (m_isServer) {
        m_isRunning = m_socket.bind(QHostAddress(m_serverIp), m_serverPort);
    } else {
        m_partAddress = QHostAddress(m_serverIp);
        m_partPort = m_serverPort;
        if (m_clientIp.isEmpty()) {
             m_isRunning = m_socket.bind();
        } else {
            m_isRunning = m_socket.bind(QHostAddress(m_clientIp));
        }
    }

    if (!m_isRunning) {
        m_lastError = "Failed to bind socket: " + m_socket.errorString();
    } else {
        connect(&m_socket, &QUdpSocket::readyRead, this, &UDPComm::readyRead);
    }

    return m_isRunning;
}

bool UDPComm::sendData(const QByteArray &data) {
    if (!m_isRunning) {
        m_lastError = "Socket is not running.";
        return false;
    }

    qint64 result = m_socket.writeDatagram(data, m_partAddress, m_partPort);

    if (result == -1) {
        m_lastError = "Failed to send data: " + m_socket.errorString();
        return false;
    }

    return true;
}

void UDPComm::stop() {
    if (m_isRunning) {
        m_socket.close();
        m_isRunning = false;
    }
}

void UDPComm::readyRead() {
    while (m_socket.hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_socket.pendingDatagramSize());

        qint64 result = m_socket.readDatagram(datagram.data(), datagram.size(),
                            &m_partAddress, &m_partPort);
        if (result == -1) {
            m_lastError = "Failed to read datagram: " + m_socket.errorString();
            return;
        }

        if (m_isServer) {
            if (!m_clientIp.isEmpty() && (m_partAddress != QHostAddress(m_clientIp))) {
                continue;
            }
        }
//        else {
//            if ((m_partAddress != QHostAddress(m_serverIp)) || (m_partPort != m_serverPort)) {
//                continue;
//            }
//        }

        emit dataReceived(datagram);
    }
}
