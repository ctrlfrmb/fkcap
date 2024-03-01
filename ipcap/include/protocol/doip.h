/**
 * @file    doip.h
 * @ingroup figkey
 * @brief   
 * @author  leiwei
 * @date    2023.12.13
 * Copyright (c) figkey 2023-2033
 */

#pragma once

#ifndef FIGKEY_PCAP_DOIP_HPP
#define FIGKEY_PCAP_DOIP_HPP

#include <iostream>
#include "def.h"

namespace figkey {
// DoIP packet parse class 
class DoIPPacketParse {
public:
    // Delete copy constructor and assignment operator to ensure uniqueness of the DoIP packet parse
    DoIPPacketParse(const DoIPPacketParse&) = delete;
    DoIPPacketParse(DoIPPacketParse&&) = delete;
    DoIPPacketParse& operator=(const DoIPPacketParse&) = delete;
    DoIPPacketParse& operator=(DoIPPacketParse&&) = delete;

    // Retrieve an instance of the DoIP packet parse(singleton pattern)
    static DoIPPacketParse& Instance() {
        static DoIPPacketParse obj;
        return obj;
    }

    bool parse(uint8_t& protocol, const std::vector<uint8_t>& packet);

private:
    // DoIP packet parse constructor
    DoIPPacketParse();

    // DoIP packet parse destructor
    ~DoIPPacketParse();
};

}  // namespace figkey

#endif // !FIGKEY_PCAP_DOIP_HPP
