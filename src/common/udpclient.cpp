/**
 * @file    udpclient.h
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
#include "common/udpclient.h"

namespace opensource {
namespace ctrlfrmb {

UDPClient::UDPClient() {
	connected = false;
}

UDPClient::~UDPClient() {
	Disconnect();
}

bool UDPClient::Connect(const char* ipAddress, int port, const char* localIP) {
	// Initialize Winsock
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
		std::cerr << "WSAStartup failed with error code: " << result << std::endl;
		return false;
	}

	// Create a UDP socket
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET) {
		std::cerr << "socket() failed with error code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}

	// Set the local IP address to bind to
	sockaddr_in localAddr;
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(0);  // Let the system assign a port
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

	connected = true;
	return true;
}

void UDPClient::Disconnect() {
	if (connected) {
		closesocket(sock);
		WSACleanup();
		connected = false;
	}
}

bool UDPClient::Send(const std::vector<uint8_t>& data) {
	if (!connected) {
		std::cerr << "Not connected, cannot send data" << std::endl;
		return false;
	}

	int result = sendto(sock, (const char*)data.data(), data.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR) {
		std::cerr << "sendto() failed with error code: " << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

}// namespace ctrlfrmb
}// namespace opensource

