#include <QNetworkProxy>
#include "common/tcpcomm.h"
#include "config.h"

TCPComm::TCPComm(const QString &clientIp, const QString &serverIp,
                 int serverPort, bool isServer, QObject *parent)
    : BaseComm(clientIp, serverIp, serverPort, isServer, parent), m_socket(nullptr)
{ }

TCPComm::~TCPComm() {
    stop();
}

bool TCPComm::start() {
    if (m_isRunning)
        return true;

    bool result = false;
    if (m_isServer) {
        m_isRunning = m_server.listen(QHostAddress(m_serverIp), m_serverPort);
        if (!m_isRunning) {
            m_lastError = "Server failed to listen: " + m_server.errorString();
        } else {
            connect(&m_server, &QTcpServer::newConnection, this, &TCPComm::newConnection);
        }
    } else {
        m_socket = new QTcpSocket(this);
        if (figkey::CaptureConfig::Instance().getConfigInfo().tcpNoProxy) {
            m_socket->setProxy(QNetworkProxy::NoProxy);
        }
        if (!m_clientIp.isEmpty() && !m_socket->bind(QHostAddress(m_clientIp))) {
            m_lastError = "Failed to bind to local IP: " + m_socket->errorString();
            return false;
        }
        m_socket->connectToHost(m_serverIp, m_serverPort);
        connect(m_socket, &QTcpSocket::readyRead, this, &TCPComm::readyRead);
        result = m_socket->waitForConnected();
        if (!result) {
            m_lastError = "Connection failed: " + m_socket->errorString();
            m_socket->deleteLater();
            m_socket = nullptr;
        }
        m_isRunning = result;
    }
    return m_isRunning;
}

bool TCPComm::sendData(const QByteArray &data) {
    if (!m_isRunning || m_socket == nullptr) {
        m_lastError = "Socket is not ready for sending data.";
        return false;
    }

    qint64 result = m_socket->write(data);

    if (result == -1) {
        m_lastError = "Failed to send data: " + m_socket->errorString();
        return false;
    }

    return true;
}

void TCPComm::stop() {
    if (m_isRunning) {
        if (m_socket) {
            m_socket->disconnectFromHost();
            m_socket->deleteLater();
            m_socket = nullptr;
        }
        if (m_server.isListening()) {
            m_server.close();
        }
        m_isRunning = false;
    }
}

void TCPComm::newConnection() {
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
    }

    m_socket = m_server.nextPendingConnection();
    if (m_socket) {
        if (!m_clientIp.isEmpty() && m_socket->peerAddress() != QHostAddress(m_clientIp)) {
            m_socket->disconnectFromHost();
            m_socket->deleteLater();
            m_socket = nullptr;
            return;
        }
        connect(m_socket, &QTcpSocket::readyRead, this, &TCPComm::readyRead);
        connect(m_socket, &QTcpSocket::disconnected, this, &TCPComm::disconnected);
    } else {
        m_lastError = "Failed to accept new connection: " + m_server.errorString();
    }
}

void TCPComm::readyRead() {
    while (m_socket && m_socket->bytesAvailable()) {
        QByteArray data = m_socket->readAll();
        emit dataReceived(data);
    }
}

void TCPComm::disconnected() {
    m_socket->deleteLater();
    m_socket = nullptr;
}
