/**
 * @file    tcpclient.h
 * @ingroup opensource::ctrlfrmb
 * @brief
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) opensource::ctrlfrmb 2024-2034
 */

#include "common/tcpclient.h"

#define DEFAULT_BUFFER_SIZE 1024

namespace opensource {
namespace ctrlfrmb {

TCPClient::TCPClient() {
	connected = false;
	recvThread = nullptr;
}

TCPClient::~TCPClient() {
	Disconnect();
	if (recvThread && recvThread->joinable()) {
		recvThread->join();
		delete recvThread;
	}
}

bool TCPClient::Connect(const char* ipAddress, int port, const char* localIP) {
    std::unique_lock<std::mutex> locker(sockMutex);
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
		closesocket(sock);
		WSACleanup();
		return false;
	}

	// Set the server address to connect to
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress, &serverAddr.sin_addr.s_addr);

	// Connect to the server
	result = connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (result != 0) {
		std::cerr << "connect() failed with error code: " << WSAGetLastError() << std::endl;
		closesocket(sock);
		WSACleanup();
		return false;
	}

	connected = true;

	// Start the receive thread
	recvThread = new std::thread(&TCPClient::ReceiveThread, this);
	return true;
}

void TCPClient::Disconnect() {
    std::unique_lock<std::mutex> locker(sockMutex);
	if (connected) {
		stopRecvThread.store(true);
		if (recvThread) {
			recvThread->join();
			delete recvThread;
			recvThread = nullptr;
		}
		closesocket(sock);
		WSACleanup();
		connected = false;
	}
}

bool TCPClient::IsConnected() {
    std::unique_lock<std::mutex> locker(sockMutex);
    return connected;
}

bool TCPClient::Send(const std::vector<uint8_t>& data) {
    if (!IsConnected()) {
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
	std::vector<uint8_t> buffer(DEFAULT_BUFFER_SIZE);
	while (!stopRecvThread) {
		int result = recv(sock, (char*)buffer.data(), buffer.size(), 0);
		if (result == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAECONNRESET) {
				// The connection was reset by the peer
				stopRecvThread = true;
			}
			else {
				std::cerr << "recv() failed with error code: " << WSAGetLastError() << std::endl;
			}
			break;
		}
		else if (result == 0) {
			// The connection was closed by the peer
			stopRecvThread = true;
			break;
		}

        if (tcpMessageCallback)
            tcpMessageCallback(buffer);

		std::vector<uint8_t> data(buffer.begin(), buffer.begin() + result);
		// Print received data in hexadecimal format
		std::cout << "Received data: ";
		for (const auto& d: data) {
			std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)d << " ";
		}
		std::cout << std::endl;
	}
}

}// namespace ctrlfrmb
}// namespace opensource

