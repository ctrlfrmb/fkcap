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

    void setCapturePacketType(CapturePacketType type);

    std::string getCapturePacketCouter(CapturePacketType type);

    std::string getCaptureLoggerDirectory(CapturePacketType type);

    // 函数：返回 TCP 或 UDP 负载的起始位置
    size_t parsePacketProtocolLength(const unsigned char* packet, const uint32_t& size, CapturePacketError& err, uint8_t& protocol);

    std::string parsePacketTimestamp(const struct timeval& ts);

    std::string parsePacketAddress(const unsigned char* packet);

    std::string parseIPPacketToHexString(const unsigned char* data, size_t length);

    std::string parsePacketDataToHexString(const std::vector<uint8_t>& data);

    IpError checkIpHeader(const ip_header* ipHeader, const size_t& packetSize);

    TcpError checkTcpHeader(const unsigned char* packet);
}  // namespace figkey

#endif // !FIGKEY_PCAP_PACKET_HPP
