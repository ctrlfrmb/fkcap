// ipcap.cpp: 定义应用程序的入口点。
//

#include "protocol/doip.h"
#include "packet.h"
#include <WinSock2.h>
#include "protocol/uds.h"
#include "common/thread_pool.hpp"

namespace figkey {

    const uint8_t DoIPHeaderLength{ 8 };
    const uint8_t DoIPGenericDoIpNackLength{ 1 };
    const uint8_t DoIPVehicleIdentificationRequestLength{ 0 };
    const uint8_t DoIPVehicleIdentificationRequestWithEIDLength{ 6 };
    const uint8_t DoIPVehicleIdentificationRequestWithVINLength{ 17 };
    const uint8_t DoIPVehicleAnnouncementLengthMin{ 32 };
    const uint8_t DoIPVehicleAnnouncementLengthMax{ 33 };
    const uint8_t DoIPRoutingActivationRequestLengthMin{ 7 };
    const uint8_t DoIPRoutingActivationRequestLengthMax{ 11 };
    const uint8_t DoIPRoutingActivationResponseLengthMin{ 9 };
    const uint8_t DoIPRoutingActivationResponseLengthMax{ 13 };
    const uint8_t DoIPAliveCheckRequestLength{ 0 };
    const uint8_t DoIPAliveCheckResponseLength{ 2 };
    const uint8_t DoIPEntityStatusRequestLength{ 0 };
    const uint8_t DoIPEntityStatusResponseLength{ 7 };
    const uint8_t DoIPPowerModeRequestLength{ 0 };
    const uint8_t DoIPPowerModeResponseLength{ 1 };
    const uint8_t DoIPDiagnosticMessageLengthMin{ 5 };
    const uint8_t DoIPDiagnosticPositiveAckLengthMin{ 5 };
    const uint8_t DoIPDiagnosticNegativeAckLengthMin{ 5 };

    // DOIP 数据包结构
    struct DoIPPacket {
        DoIPHeaderNackCode nack = { DoIPHeaderNackCode::IncorrectPatternFormat };
        uint8_t version;
        uint8_t inverseVersion;
        DoIPPayloadType payloadType;
        uint32_t payloadLength;
        std::vector<uint8_t> data;

        bool isValid() const { return DoIPHeaderNackCode::None == nack; }

        // 检查 DOIP 版本 ISO 13400-2:2012 DoIP-041
        bool checkVersion() const {
            if (version + inverseVersion != 0xFF)
                return false;
            //0xFF : default value for vehicle identification request messages
            return true;
        }

        // 校验载荷类型
        bool isValidPayloadType(DoIPPayloadType type, uint32_t len, bool isTCP) {
            switch (type) {
            case DoIPPayloadType::GenericHeaderNack:
                if (len != DoIPGenericDoIpNackLength)
                    return false;
                break;
            case DoIPPayloadType::VehicleIdentificationRequest:
                if (len != DoIPVehicleIdentificationRequestLength)
                    return false;
                if (isTCP)
                    return false;
                break;
            case DoIPPayloadType::VehicleIdentificationRequestWithEID:
                if (len != DoIPVehicleIdentificationRequestWithEIDLength)
                    return false;
                if (isTCP)
                    return false;
                break;
            case DoIPPayloadType::VehicleIdentificationRequestWithVIN:
                if (len != DoIPVehicleIdentificationRequestWithVINLength)
                    return false;
                if (isTCP)
                    return false;
                break;
            case DoIPPayloadType::VehicleIdentificationResponseOrAnnouncement:
                if ((len != DoIPVehicleAnnouncementLengthMin)
                    && (len != DoIPVehicleAnnouncementLengthMax))
                    return false;
                if (isTCP)
                    return false;
                break;
            case DoIPPayloadType::RoutingActivationRequest:
                if ((len != DoIPRoutingActivationRequestLengthMin)
                    && (len != DoIPRoutingActivationRequestLengthMax))
                    return false;
                if (!isTCP)
                    return false;
                break;
            case DoIPPayloadType::RoutingActivationResponse:
                if ((len != DoIPRoutingActivationResponseLengthMin)
                    && (len != DoIPRoutingActivationResponseLengthMax))
                    return false;
                if (!isTCP)
                    return false;
                break;
            case DoIPPayloadType::AliveCheckRequest:
                if (len != DoIPAliveCheckRequestLength)
                    return false;
                if (!isTCP)
                    return false;
                break;
            case DoIPPayloadType::AliveCheckResponse:
                if (len != DoIPAliveCheckResponseLength)
                    return false;
                if (!isTCP)
                    return false;
                break;
            case DoIPPayloadType::DoIPEntityStatusRequest:
                if (len != DoIPEntityStatusRequestLength)
                    return false;
                if (isTCP)
                    return false;
                break;
            case DoIPPayloadType::DoIPEntityStatusResponse:
                if (len != DoIPEntityStatusResponseLength)
                    return false;
                if (isTCP)
                    return false;
                break;
            case DoIPPayloadType::DiagnosticPowerModeRequest:
                if (len != DoIPPowerModeRequestLength)
                    return false;
                if (isTCP)
                    return false;
                break;
            case DoIPPayloadType::DiagnosticPowerModeResponse:
                if (len != DoIPPowerModeResponseLength)
                    return false;
                if (isTCP)
                    return false;
                break;
            case DoIPPayloadType::DiagnosticMessage:
                if (len < DoIPDiagnosticMessageLengthMin)
                    return false;
                if (!isTCP)
                    return false;
                break;
            case DoIPPayloadType::DiagnosticPositiveAck:
                if (len < DoIPDiagnosticPositiveAckLengthMin)
                    return false;
                if (!isTCP)
                    return false;
                break;
            case DoIPPayloadType::DiagnosticNegativeAck:
                if (len < DoIPDiagnosticNegativeAckLengthMin)
                    return false;
                if (!isTCP)
                    return false;
                break;
            default:
                return false;
            }
            return true;
        }

        // 解析 DOIP 数据包
        static DoIPPacket parse(const std::vector<uint8_t>& packet, bool isTCP = true) {
            DoIPPacket doipPacket;
            if (packet.size() < DoIPHeaderLength) {
                return doipPacket;
            }

            doipPacket.version = packet[0];
            doipPacket.inverseVersion = packet[1];
            doipPacket.payloadType = static_cast<DoIPPayloadType>((packet[2] << 8) | packet[3]);
            doipPacket.payloadLength = (packet[4] << 24) | (packet[5] << 16) | (packet[6] << 8) | packet[7];

            if (!doipPacket.checkVersion()) {
                doipPacket.nack = DoIPHeaderNackCode::IncorrectPatternFormat;
                return doipPacket;
            }

            if (!doipPacket.isValidPayloadType(doipPacket.payloadType, doipPacket.payloadLength, isTCP)) {
                doipPacket.nack = DoIPHeaderNackCode::UnknownPayloadType;
                return doipPacket;
            }

            if (packet.size() < DoIPHeaderLength + doipPacket.payloadLength) {
                doipPacket.nack = DoIPHeaderNackCode::InvalidPayloadLength;
                return doipPacket;
            }

            doipPacket.nack = DoIPHeaderNackCode::None;
            doipPacket.data.assign(packet.begin(), packet.begin() + DoIPHeaderLength + doipPacket.payloadLength);
            return doipPacket;
        }
    };

    DoIPPacketParse::DoIPPacketParse()
	{
	}

	DoIPPacketParse::~DoIPPacketParse()
    {
	}

	bool DoIPPacketParse::parse(uint8_t protocol, PacketLoggerInfo info, std::vector<uint8_t> packet)
	{
        static std::function<bool(DoIPPayloadType, PacketLoggerInfo, std::vector<uint8_t>)> udsParse = std::bind(&UDSPacketParse::parse, &UDSPacketParse::Instance(),
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        std::unique_lock<std::mutex> locker(mutexParse);

		bool isTCP{ true };
		// TCP 或 UDP 头解析
		if (protocol == IPPROTO_UDP)
			isTCP = false;

		auto doipPacket = DoIPPacket::parse(packet, isTCP);
		// TODO: 处理解析后的 DOIP 数据包
		if (!doipPacket.isValid())
			return false;

        std::string doipLog;
        if (info.index.empty())
            info.index = getCapturePacketCouter(CapturePacketType::DOIP);

        // 获取线程池的实例
        opensource::ctrlfrmb::ThreadPool& pool = opensource::ctrlfrmb::ThreadPool::Instance();
        std::vector<uint8_t> payload(doipPacket.data.begin()+ DoIPHeaderLength, doipPacket.data.end());
        pool.submit(udsParse, doipPacket.payloadType, info, payload);

        //if (logger)
        {
            std::string doipLog(info.index);
            doipLog += ",";

            doipLog += info.timestamp;
            doipLog += info.address;
            doipLog += parsePacketDataToHexString(doipPacket.data);
            //logger->write(doipLog);
		}

		return true;
	}
}
