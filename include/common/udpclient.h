/**
 * @file    udpclient.h
 * @ingroup opensource::ctrlfrmb
 * @brief
 * @author  leiwei
 * @date    2024.01.18
 * Copyright (c) opensource::ctrlfrmb 2024-2034
 */

#pragma once

#ifndef OPEN_SOURCE_UDP_CLIENT_H
#define OPEN_SOURCE_UDP_CLIENT_H

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <thread>
#include <iomanip>

namespace opensource {
namespace ctrlfrmb {

    class UDPClient {
    private:
        SOCKET sock;
        sockaddr_in serverAddr;
        bool connected;

    public:
        UDPClient() ;

        ~UDPClient() ;
		
        bool Connect(const char* ipAddress, int port, const char* localIP) ;

        void Disconnect() ;

        bool Send(const std::vector<uint8_t>& data) ;
    };

}// namespace ctrlfrmb
}// namespace opensource

#endif // !OPEN_SOURCE_UDP_CLIENT_H
