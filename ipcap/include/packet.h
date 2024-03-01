/**
 * @file    packet.h
 * @ingroup figkey
 * @brief   
 * @author  leiwei
 * @date    2023.12.19
 * Copyright (c) figkey 2023-2033
 */

#pragma once

#ifndef FIGKEY_PCAP_PACKET_HPP
#define FIGKEY_PCAP_PACKET_HPP

#include <vector>
#include <string>
#include "def.h"

namespace figkey {

    std::string parsePacketTimestamp(const struct timeval& ts);

    std::string parsePayloadToHexString(const std::vector<uint8_t>& data);

    PacketInfo parseIpPacket(const unsigned char* packet, const uint32_t& size);

    uint8_t checkIpHeader(const ip_header* ipHeader, const size_t& packetSize);

    uint8_t checkTcpHeader(const unsigned char* packet);
}  // namespace figkey

#endif // !FIGKEY_PCAP_PACKET_HPP
