// ipcap.cpp: 定义应用程序的入口点。
//

#include "protocol/ip.h"
#include "protocol/doip.h"
#include "common/thread_pool.hpp"
#include "packet.h"
#include "config.h"

namespace figkey {

    IPPacketParse::IPPacketParse():packetCallBack(nullptr)
	{
	}

	IPPacketParse::~IPPacketParse()
    {
	}

    void IPPacketParse::setCallback(PacketCallback callback)
    {
        packetCallBack = callback;
    }

    bool IPPacketParse::checkFilterInfo(const PacketInfo& packet, const FilterInfo& filter) {
        if (!filter.srcIP.empty() && filter.srcIP != packet.srcIP) return false;
        if (!filter.destIP.empty() && filter.destIP != packet.destIP) return false;
        if (!filter.srcMAC.empty() && filter.srcMAC != packet.srcMAC) return false;
        if (!filter.destMAC.empty() && filter.destMAC != packet.destMAC) return false;
        if (filter.srcPort != 0 && filter.srcPort != packet.srcPort) return false;
        if (filter.destPort != 0 && filter.destPort != packet.destPort) return false;

        // 当滤波器的 minLen 不为零时做最小长度过滤
        if (filter.minLen != 0) {
            if (packet.payloadLength < filter.minLen) return false;
        }
        // 当滤波器的 maxLen 不为零时做最大长度过滤
        if (filter.maxLen != 0) {
            if (packet.payloadLength > filter.maxLen) return false;
        }

        return true;
    }

    bool IPPacketParse::checkFilterProtocol(uint8_t protocl, const FilterInfo& filter) {
        if (PROTOCOL_TYPE_DEFAULT == filter.protocolType)
            return true;

        if (protocl == filter.protocolType)
            return true;

        if ((PROTOCOL_TYPE_DOIP == filter.protocolType) && (protocl >= filter.protocolType))
            return true;
        return false;
    }

    bool IPPacketParse::checkPacket(const struct pcap_pkthdr* pkthdr, PacketInfo&& info, std::vector<uint8_t>&& payload) {

        if (!checkFilterInfo(info, CaptureConfig::Instance().getConfigInfo().filter)) {
            //std::cout << "filter " << info.srcIP << " -> "<< info.destIP << std::endl;
            return false;
        }

        if (!payload.empty())
            DoIPPacketParse::Instance().parse(info.protocolType, payload);
        if (!checkFilterProtocol(info.protocolType, CaptureConfig::Instance().getConfigInfo().filter)){
            //std::cout << "filter by protocol " << info.srcIP << " -> "<< info.destIP <<", "<<static_cast<int>(info.payloadLength)<< std::endl;
            //if (!payload.empty())
            //    std::cout << parsePayloadToHexString(payload) << std::endl;
            return false;
        }

        info.timestamp = parsePacketTimestamp(pkthdr->ts);
        info.data = parsePayloadToHexString(payload);

        if (packetCallBack) {
            opensource::ctrlfrmb::ThreadPool& pool = opensource::ctrlfrmb::ThreadPool::Instance();
            pool.submit(packetCallBack, info);
        }

        return true;
    }

    bool IPPacketParse::parse(const struct pcap_pkthdr* pkthdr, const u_char* packet)
    {
        uint32_t offset {0};
        uint32_t len {pkthdr->caplen};

        while (len >= ETHERNET_IP_UDP_HEADER_MIN)
        {
            PacketInfo info = parseIpPacket(packet+offset, len);
            offset += static_cast<uint32_t>(info.index);
            std::vector<uint8_t> payload(packet+offset, packet+(offset+info.payloadLength));
            offset += info.payloadLength;
            len -= offset;
            if (info.protocolType < PROTOCOL_TYPE_TCP) {
                std::cerr << "Fatal error: " << info.data << std::endl;
                continue;
            }

            checkPacket(pkthdr, std::move(info), std::move(payload));
        }

        if (len != 0) {
            //cache.clear();
            //if ((offset+len) != pkthdr->caplen)
                //cache.assign(packet+offset, packet+(pkthdr->caplen-1));
            //else
                //std::cerr << "Fatal error: IP package is incomplete!!! remaining length: "<<len <<" capture length: "<<pkthdr->caplen<<" must length: " <<pkthdr->len << std::endl;
            return false;
        }

        return true;
    }
}
