/**
 * @file    uds.h
 * @ingroup figkey
 * @brief   
 * @author  leiwei
 * @date    2023.12.19
 * Copyright (c) figkey 2023-2033
 */

#pragma once

#ifndef FIGKEY_PCAP_UDS_HPP
#define FIGKEY_PCAP_UDS_HPP

#include <vector>
#include <mutex>
#include "def.h"

namespace figkey {
    // UDS packet parse class 
    class UDSPacketParse {
    public:
        // Delete copy constructor and assignment operator to ensure uniqueness of the UDS packet parse
        UDSPacketParse(const UDSPacketParse&) = delete;
        UDSPacketParse(UDSPacketParse&&) = delete;
        UDSPacketParse& operator=(const UDSPacketParse&) = delete;
        UDSPacketParse& operator=(UDSPacketParse&&) = delete;

        // Retrieve an instance of the UDS packet parse(singleton pattern)
        static UDSPacketParse& Instance() {
            static UDSPacketParse obj;
            return obj;
        }

        bool parse(DoIPPayloadType type, PacketLoggerInfo info, std::vector<uint8_t> packet);

    private:
        std::mutex mutexParse;

        // UDS packet parse constructor
        UDSPacketParse();

        // UDS packet parse destructor
        ~UDSPacketParse();
    };

}  // namespace figkey

#endif // !FIGKEY_PCAP_UDS_HPP
