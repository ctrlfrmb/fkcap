/**
 * @file    ip.h
 * @ingroup figkey
 * @brief   
 * @author  leiwei
 * @date    2023.12.20
 * Copyright (c) figkey 2023-2033
 */

#pragma once

#ifndef FIGKEY_PCAP_IP_HPP
#define FIGKEY_PCAP_IP_HPP

#include <mutex>
#include <winsock2.h>
#include "def.h"

namespace figkey {
    // IP packet parse class 
    class IPPacketParse {
    public:
        // Delete copy constructor and assignment operator to ensure uniqueness of the IP packet parse
        IPPacketParse(const IPPacketParse&) = delete;
        IPPacketParse(IPPacketParse&&) = delete;
        IPPacketParse& operator=(const IPPacketParse&) = delete;
        IPPacketParse& operator=(IPPacketParse&&) = delete;

        // Retrieve an instance of the IP packet parse(singleton pattern)
        static IPPacketParse& Instance() {
            static IPPacketParse obj;
            return obj;
        }

        bool parse(unsigned char* buf, struct timeval ts, unsigned int captureLen, unsigned int packetLen);

    private:
        std::mutex mutexParse;

        // IP packet parse constructor
        IPPacketParse();

        // IP packet parse destructor
        ~IPPacketParse();

        bool writeCapturePacketError(CapturePacketError err, unsigned char* buf, struct timeval ts, unsigned int packetLen);

        uint32_t checkInput(unsigned char* buf, struct timeval ts, unsigned int captureLen, unsigned int len);
    };

}  // namespace figkey

#endif // !FIGKEY_PCAP_IP_HPP
