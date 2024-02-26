// ipcap.cpp: 定义应用程序的入口点。
//

#include "protocol/ip.h"
#include "protocol/doip.h"
#include "common/thread_pool.hpp"
#include "packet.h"
#include "config.h"

namespace figkey {

    IPPacketParse::IPPacketParse()
	{
	}

	IPPacketParse::~IPPacketParse()
    {
	}

	bool IPPacketParse::writeCapturePacketError(CapturePacketError err, unsigned char* buf, struct timeval ts, unsigned int len) {
		if (err == CapturePacketError::NoError)
			return false;

        std::string ipLog(getCapturePacketCouter(CapturePacketType::DEFAULT));
        ipLog += parsePacketTimestamp(ts);
        switch (err)
        {
        case CapturePacketError::SystemError:
            ipLog += "[SystemError],";
            break;
        case CapturePacketError::IpHeaderLostError:
            ipLog += "[IpHeaderLostError],";
            break;
        case CapturePacketError::TcpHeaderLostError:
            ipLog += "[TcpHeaderLostError],";
            break;
        case CapturePacketError::TcpHeaderOffsetError:
            ipLog += "[TcpHeaderLostError],";
            break;
        case CapturePacketError::UdpHeaderLostError:
            ipLog += "[TcpHeaderLostError],";
            break;
        default:
            ipLog += "[ProtocolHeaderLostError],";
            break;
        }

        ipLog += parseIPPacketToHexString(buf, len);
        //logger->write(ipLog);
        printf("%s\n", ipLog.c_str());

		if (buf)
			delete[] buf;
        return false;
    }

	uint32_t IPPacketParse::checkInput(unsigned char* buf, struct timeval ts, unsigned int captureLen, unsigned int packetLen) {
        std::string err(PACKET_LOGGER_ERROR);
		if (captureLen == 0)
			err = "[SnapLengthError]: capture length is 0";
        else if (captureLen > CAPTURE_SNAP_LENGTH)
			err = "[SnapLengthError]: capture length more than 65535";
		//else if (captureLen < packetLen)
		//	err == "[SnapLengthError]: capture length less than packet length";
		else
			return captureLen;

		if (buf)
			delete[] buf;
        printf("snap length error: capture length: %u, valid lenght: %u, capture max length: %u\n", captureLen, packetLen, CAPTURE_SNAP_LENGTH);

		std::unique_lock<std::mutex> locker(mutexParse);
        {
			std::string ipLog(getCapturePacketCouter(CapturePacketType::DEFAULT));
			ipLog += parsePacketTimestamp(ts);
			ipLog += err;
            //logger->write(ipLog);
		}

		return 0;
	}

	bool IPPacketParse::parse(unsigned char* buf, struct timeval ts, unsigned int captureLen, unsigned int packetLen)
	{
		static std::function<bool(uint8_t, PacketLoggerInfo, std::vector<uint8_t>)> doipParse = std::bind(&DoIPPacketParse::parse, &DoIPPacketParse::Instance(),
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		
        auto capLen = checkInput(buf, ts, captureLen, packetLen);
		if (capLen <= 0 )
			return false;

		size_t len{ 0 };
		CapturePacketError err{ CapturePacketError::NoError };
		uint8_t protocol{ 0 };

		if (0 == (len = parsePacketProtocolLength(buf, capLen, err, protocol)))
			return writeCapturePacketError(err, buf, ts, capLen);

		std::unique_lock<std::mutex> locker(mutexParse);
		PacketLoggerInfo info;
        //if (logger)
			info.index = getCapturePacketCouter(CapturePacketType::DEFAULT);
		info.timestamp = parsePacketTimestamp(ts);
		info.address = parsePacketAddress(buf);

		if (!CaptureConfig::Instance().checkFilterAddress(info.address)) {
			//// 获取 IP 头部的起始位置
			const ip_header* ip = reinterpret_cast<const ip_header*>(buf + sizeof(ethernet_header));
			auto result = checkIpHeader(ip, capLen - sizeof(ethernet_header));
			std::cout << "check ip header result: " <<static_cast<int>(result )<< std::endl;

			if (protocol == IPPROTO_TCP) {
				auto result = checkTcpHeader(buf);
				std::cout << "check tcp header result: " << static_cast<int>(result) << std::endl;
			}

			if (len < capLen) {
				std::vector<uint8_t> payload(buf + len, buf + capLen);
				// 获取线程池的实例
				opensource::ctrlfrmb::ThreadPool& pool = opensource::ctrlfrmb::ThreadPool::Instance();
				pool.submit(doipParse, protocol, info, payload);
			}

            //if (logger)
            {
				std::string ipLog(info.index);
				ipLog += info.timestamp;
				ipLog += info.address;
				ipLog += parseIPPacketToHexString(buf, capLen);
                //logger->write(ipLog);
			}
		}

		if (buf)
			delete[] buf;
		return true;
	}
}
