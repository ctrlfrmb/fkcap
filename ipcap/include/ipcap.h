/**
 * @file    ipcap.h
 * @ingroup figkey
 * @brief   
 * @author  leiwei
 * @date    2023.12.12
 * Copyright (c) figkey 2023-2033
 */

#pragma once

#ifndef FIGKEY_PCAP_COM_HPP
#define FIGKEY_PCAP_COM_HPP

#include "pcap.h"
#include "def.h"

namespace figkey {

class NpcapCom {
public:
    explicit NpcapCom();

    ~NpcapCom();

    std::vector<NetworkInfo> getNetworkList();

    bool setCaptureNetwork(const std::string& networkName);

    bool setPacketFilter(const std::string& filterExpression, uint32_t netmask = PCAP_NETMASK_UNKNOWN);

    void startCapture(CapturePacketType type, bool isSave);

    void asyncStartCapture(CapturePacketType type, bool isSave);

    void stopCapture();

private:
    pcap_t* handle;

    void init();

    static void pcapHandler(u_char* user, const struct pcap_pkthdr* pkthdr, const u_char* packet);
};

}  // namespace figkey

#endif // !FIGKEY_PCAP_COM_HPP
