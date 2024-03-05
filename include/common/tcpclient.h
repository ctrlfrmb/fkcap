/**
 * @file    tcpclient.h
 * @ingroup opensource::ctrlfrmb
 * @brief   
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) opensource::ctrlfrmb 2024-2034
 */

#pragma once

#ifndef OPEN_SOURCE_TCP_CLIENT_H
#define OPEN_SOURCE_TCP_CLIENT_H

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <iomanip>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace opensource {
namespace ctrlfrmb {

    using HandleTCPMessage = std::function<void(std::vector<uint8_t>)>;

    class TCPClient {
    private:
        SOCKET sock{ INVALID_SOCKET };
        sockaddr_in serverAddr;
        std::atomic<bool> connected{ false };

        std::thread* recvThread{ nullptr };
        std::atomic<bool> stopRecvThread{ false };

    public:
        TCPClient() ;

        ~TCPClient() ;

        bool Connect(const char* ipAddress, int port, const char* localIP) ;
		
        void Disconnect() ;

        bool IsConnected();

        bool Send(const std::vector<uint8_t>& data) ;

        void SetMessageCallback(const HandleTCPMessage& callback);

    private:
        HandleTCPMessage tcpMessageCallback{ nullptr };

        bool AbnormalShutdown();

        void ReceiveThread() ;
    };

}// namespace ctrlfrmb
}// namespace opensource

#endif // !OPEN_SOURCE_TCP_CLIENT_H
