/**
 * @file    tcpclient.h
 * @ingroup opensource::ctrlfrmb
 * @brief
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) opensource::ctrlfrmb 2024-2034
 */
#ifdef _WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x6000
#include <ws2tcpip.h>
#endif
#include "common/tcpclient.h"

#define DEFAULT_BUFFER_SIZE 1024

namespace opensource {
namespace ctrlfrmb {

TCPClient::TCPClient() {
}

TCPClient::~TCPClient() {
	Disconnect();
	if (recvThread && recvThread->joinable()) {
		recvThread->join();
		delete recvThread;
        recvThread = nullptr;
	}
}

bool TCPClient::Connect(const char* ipAddress, int port, const char* localIP) {
    if (connected) {
        return true;
    }

	// Initialize Winsock
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cerr << "WSAStartup failed with error code: " << result << std::endl;
		return false;
	}

	// Create a TCP socket
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		std::cerr << "socket() failed with error code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}

	// Set the local IP address to bind to
	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(0); // Let the system assign a port
	if (localIP != nullptr) {
		inet_pton(AF_INET, localIP, &localAddr.sin_addr.s_addr);
	}
	else {
		localAddr.sin_addr.s_addr = INADDR_ANY;
	}

	result = bind(sock, (sockaddr*)&localAddr, sizeof(localAddr));
	if (result != 0) {
		std::cerr << "bind() failed with error code: " << WSAGetLastError() << std::endl;
        return AbnormalShutdown();
	}

	// Set the server address to connect to
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress, &serverAddr.sin_addr.s_addr);

	// Connect to the server
	result = connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (result != 0) {
		std::cerr << "connect() failed with error code: " << WSAGetLastError() << std::endl;
        return AbnormalShutdown();
	}

	connected = true;
    stopRecvThread.store(false);

	// Start the receive thread
    if (nullptr == recvThread) {
        recvThread = new std::thread(&TCPClient::ReceiveThread, this);
    }

	return true;
}

bool TCPClient::AbnormalShutdown() {
    if (INVALID_SOCKET != sock) {
        int result = closesocket(sock);
        if (result == 0) {
            WSACleanup();
        } else {
            // 错误处理
            std::cerr << "closesocket failed with error: " << WSAGetLastError() << std::endl;
        }
        sock = INVALID_SOCKET;
    }

    return false;
}

void TCPClient::Disconnect() {
    stopRecvThread.store(true);

    AbnormalShutdown();

    if (recvThread) {
        if (recvThread->joinable())
            recvThread->join();
        delete recvThread;
        recvThread = nullptr;
    }

    connected = false;
}

bool TCPClient::IsConnected() {
    return connected;
}

bool TCPClient::Send(const std::vector<uint8_t>& data) {
    if (!connected) {
		return false;
	}

	int result = send(sock, reinterpret_cast<const char*>(data.data()), data.size(), 0);
	if (result == SOCKET_ERROR) {
		std::cerr << "send() failed with error code: " << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

void TCPClient::SetMessageCallback(const HandleTCPMessage& callback) {
    tcpMessageCallback = callback;
}

void TCPClient::ReceiveThread() {
    uint8_t buffer[DEFAULT_BUFFER_SIZE]{0};
	while (!stopRecvThread) {
        int result = recv(sock, (char*)buffer, DEFAULT_BUFFER_SIZE, 0);
		if (result == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAECONNRESET) {
				// The connection was reset by the peer
				stopRecvThread = true;
                std::cerr << "The connection was reset by the peer " << std::endl;
			}
			else {
				std::cerr << "recv() failed with error code: " << WSAGetLastError() << std::endl;
			}
			break;
		}
		else if (result == 0) {
			// The connection was closed by the peer
			stopRecvThread = true;
            std::cerr << "The connection was closed by the peer " << std::endl;
			break;
		}

        if (tcpMessageCallback) {
            std::vector<uint8_t> data(buffer, buffer + result);
            tcpMessageCallback(data);
        }
    }
}

}// namespace ctrlfrmb
}// namespace opensource

