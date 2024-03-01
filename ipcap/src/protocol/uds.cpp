// ipcap.cpp: 定义应用程序的入口点。
//

#include <unordered_set>
#include "protocol/uds.h"
#include "packet.h"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace figkey {

    const uint8_t DoIPUDSHeaderLength{ 4 };

    // UDS 检查器类
    class UDSChecker {
    public:
        // 初始化服务 SID 表，包含图片中的所有有效 SID
        static const std::unordered_set<uint8_t> validSids;
        // 检查是否为 UDS 数据
        static bool isUdsData(DoIPPayloadType payloadType, const std::vector<uint8_t>& data) {
            // 这里可以添加更详细的 UDS 数据检查逻辑
            switch (payloadType) {
            case DoIPPayloadType::DiagnosticMessage:
                // 这里进行 UDS 数据格式的具体校验
                if (data.empty() || (data.size() <= DoIPUDSHeaderLength)) {
                    return false;
                }
                return true;
            default:
                break;
            }

            return false;
        }

    private:

        // 检查数据是否为 UDS 数据
        static bool checkUdsDataFormat(const std::vector<uint8_t>& data) {
            std::cout<<"uds: "<< parsePayloadToHexString(data)<<std::endl;
            if (data.empty() || (data.size() <= DoIPUDSHeaderLength)) {
                return false;
            }
            uint8_t sid = data[DoIPUDSHeaderLength];  // 第一个字节是 SID
            return validSids.find(sid) != validSids.end();
        }
    };

    // 初始化服务 SID 表
    const std::unordered_set<uint8_t> UDSChecker::validSids = {
        0x10, // Diagnostic Session Control
        0x11, // ECU Reset
        0x27, // Security Access
        // ... 更多的 SID
        0x86, // Response On Event
        0x87, // Link Control
        0x22, // Read Data By Identifier
        // ... 更多的 SID
        0x3E, // Tester Present
        0x85, // Control DTC Setting
        // ... 更多的 SID
        0x14, // Clear Diagnostic Information
        0x19, // Read DTC Information
        // ... 更多的 SID
        0x31, // Routine Control
        0x34, // Request Download
        // ... 更多的 SID
        0x36, // Transfer Data
        0x37, // Request Transfer Exit
        0x38  // Request File Transfer
        // 注意：列表需要根据图片中的全部 SID 完整填写
    };

    UDSPacketParse::UDSPacketParse()
	{
	}

    UDSPacketParse::~UDSPacketParse()
    {
	}

    bool UDSPacketParse::parse(DoIPPayloadType type, const std::vector<uint8_t>& packet)
    {
		if (!UDSChecker::isUdsData(type, packet))
			return false;

		return true;
	}
}
