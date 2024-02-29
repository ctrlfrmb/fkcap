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

	bool IPPacketParse::parse(unsigned char* buf, struct timeval ts, unsigned int captureLen, unsigned int packetLen)
	{
		static std::function<bool(uint8_t, PacketLoggerInfo, std::vector<uint8_t>)> doipParse = std::bind(&DoIPPacketParse::parse, &DoIPPacketParse::Instance(),
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		
        PacketInfo info = parseIpPacket(buf, captureLen);
        info.timestamp = parsePacketTimestamp(ts);
        if (buf)
            delete[] buf;

        std::unique_lock<std::mutex> locker(mutexParse);
        if (packetCallBack) {
            if (info.protocolType >= PROTOCOL_TYPE_TCP)
                info.index = ++packetCounter;
            packetCallBack(info);
        }
//		if (!CaptureConfig::Instance().checkFilterAddress(info.address)) {
//			//// 获取 IP 头部的起始位置
//			const ip_header* ip = reinterpret_cast<const ip_header*>(buf + sizeof(ethernet_header));
//			auto result = checkIpHeader(ip, capLen - sizeof(ethernet_header));
//			std::cout << "check ip header result: " <<static_cast<int>(result )<< std::endl;

//			if (protocol == IPPROTO_TCP) {
//				auto result = checkTcpHeader(buf);
//				std::cout << "check tcp header result: " << static_cast<int>(result) << std::endl;
//			}

//			if (len < capLen) {
//				std::vector<uint8_t> payload(buf + len, buf + capLen);
//				// 获取线程池的实例
//				opensource::ctrlfrmb::ThreadPool& pool = opensource::ctrlfrmb::ThreadPool::Instance();
//				pool.submit(doipParse, protocol, info, payload);
//			}

//            //if (logger)
//            {
//				std::string ipLog(info.index);
//				ipLog += info.timestamp;
//				ipLog += info.address;
//				ipLog += parseIPPacketToHexString(buf, capLen);
//                //logger->write(ipLog);
//			}
//		}

		return true;
	}
}
