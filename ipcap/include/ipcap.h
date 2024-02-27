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
    // Delete copy constructor and assignment operator to ensure uniqueness of the npcap common class
    NpcapCom(const NpcapCom&) = delete;
    NpcapCom(NpcapCom&&) = delete;
    NpcapCom& operator=(const NpcapCom&) = delete;
    NpcapCom& operator=(NpcapCom&&) = delete;

    // Retrieve an instance of the packet capture class(singleton pattern)
    static NpcapCom& Instance() {
        static NpcapCom obj;
        return obj;
    }

    std::vector<NetworkInfo> getNetworkList();

    bool setCaptureNetwork(const std::string& networkName);

    bool setPacketFilter(const std::string& filterExpression, uint32_t netmask = PCAP_NETMASK_UNKNOWN);

    void startCapture(ProtocolType type, bool isSave);

    void asyncStartCapture(ProtocolType type, bool isSave);

    void stopCapture();

private:
    pcap_t* handle;

    NpcapCom();

    ~NpcapCom();

    void init();

    static void pcapHandler(u_char* user, const struct pcap_pkthdr* pkthdr, const u_char* packet);
};

}  // namespace figkey

#endif // !FIGKEY_PCAP_COM_HPP
