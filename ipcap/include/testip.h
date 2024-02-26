/**
 * @file    testip.h
 * @ingroup figkey
 * @brief   
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) figkey 2024-2034
 */

#pragma once

#ifndef FIGKEY_TEST_IP_H
#define FIGKEY_TEST_IP_H

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <iomanip>

#define DEFAULT_BUFFER_SIZE 1024

namespace figkey {

    class UDPClient {
    private:
        SOCKET sock;
        sockaddr_in serverAddr;
        bool connected;

    public:
        UDPClient() {
            connected = false;
        }

        ~UDPClient() {
            Disconnect();
        }

        bool Connect(const char* ipAddress, int port, const char* localIP) {
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

        void Disconnect() {
            if (connected) {
                closesocket(sock);
                WSACleanup();
                connected = false;
            }
        }

        bool Send(const std::vector<uint8_t>& data) {
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
    };

    class TCPClient {
    private:
        SOCKET sock;
        sockaddr_in serverAddr;
        bool connected;

        std::thread* recvThread;
        std::atomic<bool> stopRecvThread{ false };
        std::mutex recvMutex;
        std::condition_variable recvCondVar;
        std::queue<std::vector<uint8_t>> recvQueue;

    public:
        TCPClient() {
            connected = false;
            recvThread = nullptr;
        }

        ~TCPClient() {
            Disconnect();
            if (recvThread && recvThread->joinable()) {
                recvThread->join();
                delete recvThread;
            }
        }

        bool Connect(const char* ipAddress, int port, const char* localIP) {
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

        void Disconnect() {
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

        bool Send(const std::vector<uint8_t>& data) {
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

        std::vector<uint8_t> Receive() {
            std::unique_lock<std::mutex> lock(recvMutex);
            recvCondVar.wait(lock, [this] { return !recvQueue.empty() || stopRecvThread; });

            if (stopRecvThread) {
                return std::vector<uint8_t>();
            }

            std::vector<uint8_t> data = std::move(recvQueue.front());
            recvQueue.pop();

            return data;
        }
    private:
        void ReceiveThread() {
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

                std::vector<uint8_t> data(buffer.begin(), buffer.begin() + result);
                // Print received data in hexadecimal format
                std::cout << "Received data: ";
                for (const auto& d: data) {
                    std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)d << " ";
                }
                std::cout << std::endl;

                // Push the received data to the queue
                std::lock_guard<std::mutex> lock(recvMutex);
                recvQueue.push(std::move(data));
                recvCondVar.notify_one();
            }
        }
    };

}// namespace figkey

#endif // !FIGKEY_TEST_IP_H
