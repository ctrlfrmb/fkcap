// ipcap.cpp: 定义应用程序的入口点。
//
#include <iomanip>
#ifdef _WIN32
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#include <sstream>
#include <atomic>
#include <chrono>
#endif // _WIN32
#include "def.h"
#include "packet.h"
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

namespace figkey {
    static std::atomic<uint64_t> packetLogCounter{ 0 };
    static ProtocolType packetCaptureType{ ProtocolType::DEFAULT };

    const uint8_t ipPacketMinLength{ sizeof(ethernet_header) + sizeof(ip_header)};

    void setProtocolType(ProtocolType type)
    {
        packetCaptureType = type;
    }

    std::string getCapturePacketCouter(ProtocolType type)
    {
        if (type == packetCaptureType) {
            ++packetLogCounter;
        }

        return std::to_string(packetLogCounter) + ",";
    }

    std::string getCaptureLoggerDirectory(ProtocolType type)
    {
        // 获取当前系统时间
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);

        // 使用 localtime_s 而不是 localtime
        std::tm timeBuffer;
        localtime_s(&timeBuffer, &now_c);
        // 使用 stringstream 进行格式化
        std::stringstream ss;
        ss << "fkcap/";
        ss << std::put_time(&timeBuffer, "%Y%m%d");
        switch (type)
        {
        case ProtocolType::UDS:
            ss << "/uds";
            break;
        case ProtocolType::DOIP:
            ss << "/doip";
            break;
        default:
            ss << "/ip";
            break;
        }
        return ss.str();
    }

    // 函数：返回 TCP 或 UDP 负载的起始位置
    size_t parsePacketProtocolLength(const unsigned char* packet, const uint32_t& size, CapturePacketError& err, uint8_t& protocol) {
        if (size < ipPacketMinLength) {
            if (size > 0) {
                printf("ip header error: capture length:%d, ip header min length:%d \n", size, ipPacketMinLength);
                err = CapturePacketError::IpHeaderLostError;
            }
            else 
                err = CapturePacketError::NoError;
            return 0;
        }

        // 获取 IP 头部的起始位置
        const ip_header* ip = reinterpret_cast<const ip_header*>(packet + sizeof(ethernet_header));
        if (!ip) {
            printf("ip header error for system \n");
            err = CapturePacketError::SystemError;
            return 0;
        }

        // 计算 IP 头部的长度
        //uint8_t version = (ip->ihl_and_version >> 4) & 0xF;
        size_t ipHeaderLen = (ip->ihl_and_version & 0xF) * 4;

        // 获取协议类型 (TCP 或 UDP)
        size_t protocolHeaderLen = 0;
        if (ip->iph_protocol == IPPROTO_TCP) {
            // TCP 头的长度 (可能包括选项)
            auto len = sizeof(ethernet_header) + ipHeaderLen + sizeof(tcp_header);
            if (size < len) {
                printf("tcp header error: capture length:%d, tcp header min length:%zu \n", size, len);
                err = CapturePacketError::TcpHeaderLostError;
                return 0;
            }

            const tcp_header* tcp = reinterpret_cast<const tcp_header*>(packet + sizeof(ethernet_header) + ipHeaderLen);
            if (!tcp) {
                printf("tcp header error for system \n");
                err = CapturePacketError::SystemError;
                return 0;
            }

            protocolHeaderLen = ((tcp->data_offset_and_reserved >> 4) & 0xF) * 4; // 提取 th_off 的值
            if (protocolHeaderLen  == 0) {
                printf("tcp header error: header length is 0 and  data_offset_and_reserved %d \n", tcp->data_offset_and_reserved);
                err = CapturePacketError::UdpHeaderLostError;
                return 0;
            }
        }
        else if (ip->iph_protocol == IPPROTO_UDP) {
            auto len = sizeof(ethernet_header) + ipHeaderLen + sizeof(udp_header);
            if (size < len) {
                printf("udp header error: capture length:%d, udp header min length:%zu \n", size, len);
                err = CapturePacketError::UdpHeaderLostError;
                return 0;
            }

            // UDP 头的长度是固定的
            protocolHeaderLen = sizeof(udp_header);
        }
        else {
            err = CapturePacketError::NoError;
            return 0;
        }

        auto len = sizeof(ethernet_header) + ipHeaderLen + protocolHeaderLen;
        if (size < len) {
            printf("protocol header error: capture length:%d, protocol header min length:%zu \n", size, len);
            err = CapturePacketError::ProtocolHeaderLostError;
            return 0;
        }

        // 返回负载数据的起始位置
        protocol = ip->iph_protocol;
        return len;
    }

    std::string parsePacketTimestamp(const struct timeval& ts) {
        struct tm ltime;
        char timestr[16];
        time_t local_tv_sec;

        // 确保 tv_sec 是合理的时间戳
        if (ts.tv_sec < 0) {
            // 处理无效时间戳
            // 此处可以记录错误或选择一个默认值
            local_tv_sec = 0;
        }
        else {
            local_tv_sec = ts.tv_sec;
        }

        // 使用 localtime_s 而不是 localtime
        errno_t err = localtime_s(&ltime, &local_tv_sec);
        if (err != 0) {
            // 处理 localtime_s 的错误
            // 此处可以记录错误或选择一个默认值
            return "00:00:00.000,";
        }

        strftime(timestr, sizeof timestr, "%H:%M:%S", &ltime);
        std::string str(timestr);

        // 构建时间戳字符串
        str += ".";
        str += std::to_string(ts.tv_usec);

        return str;
    }

    std::string parsePacketAddress(const unsigned char* packet) {
        std::stringstream ss;

        // 以太网帧头解析
        auto* eth = reinterpret_cast<const ethernet_header*>(packet);
        ss << "mac[";
        for (int i = 0; i < 6; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(eth->src_mac[i]);
            if (i < 5) ss << ":";
        }

        ss << "->";
        for (int i = 0; i < 6; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(eth->dest_mac[i]);
            if (i < 5) ss << ":";
        }
        ss << "],";

        // IP 头解析
        auto* ip = reinterpret_cast<const ip_header*>(packet + sizeof(ethernet_header));

        ss << "ip[";
        char sourceIp[INET_ADDRSTRLEN];  // INET_ADDRSTRLEN 用于 IPv4 地址
        char destIp[INET_ADDRSTRLEN];

        // 使用 inet_ntop 转换 IP 地址
        //inet_ntop(AF_INET, &ip->iph_sourceip, sourceIp, INET_ADDRSTRLEN);
        uint32_t non_const_iph_sourceip = ip->iph_sourceip;
        inet_ntop(AF_INET, &non_const_iph_sourceip, sourceIp, INET_ADDRSTRLEN);

        //inet_ntop(AF_INET, &ip->iph_destip, destIp, INET_ADDRSTRLEN);
        uint32_t non_const_iph_destip = ip->iph_destip;
        inet_ntop(AF_INET, &non_const_iph_destip, destIp, INET_ADDRSTRLEN);

        ss << sourceIp << "->" << destIp;
        ss << "],";

        // TCP 或 UDP 头解析
        ss << std::dec;
        uint8_t ihl = ip->ihl_and_version & 0xF;
        if (ip->iph_protocol == IPPROTO_TCP) {
            ss << "tcp[";
            auto* tcp = reinterpret_cast<const tcp_header*>(packet + sizeof(ethernet_header) + (ihl * 4));
            ss << ntohs(tcp->th_sport);
            ss << "->" << ntohs(tcp->th_dport);
            ss << "],";
        }
        else if (ip->iph_protocol == IPPROTO_UDP) {
            ss << "udp[";
            auto* udp = reinterpret_cast<const udp_header*>(packet + sizeof(ethernet_header) + (ihl * 4));
            ss << ntohs(udp->uh_sport);
            ss << "->" << ntohs(udp->uh_dport);
            ss << "],";
        }

        return ss.str();
    }

    std::string parseIPPacketToHexString(const unsigned char* data, size_t length) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        for (size_t i = 0; i < length; ++i) {
            ss << std::setw(2) << static_cast<unsigned>(data[i]) << " ";
        }

        return ss.str();
    }

    std::string parsePacketDataToHexString(const std::vector<uint8_t>& data) {
        std::stringstream ss;
        ss << "len[" << data.size() << "],";
        ss << std::hex << std::setfill('0');

        for (const auto& d : data)
            ss << std::setw(2) << static_cast<unsigned>(d) << " ";

        return ss.str();
    }

    PacketInfo parsePacketProtocolLength(const unsigned char* packet, const uint32_t& size) {
        PacketInfo info;
        std::stringstream ss;
        info.len = size;
        info.protocolType = static_cast<uint8_t>(ProtocolType::DEFAULT);

        if (size < ipPacketMinLength) {
            if (size > 0) {
                ss << "[SnapLengthError]: capture length " << size << " less than ip header min length "
                   << ipPacketMinLength << ", ";
                info.info = ss.str()+parseIPPacketToHexString(buf, size);
            }
            else
                info.info  = "[SnapLengthError]: capture length is 0";
            return info;
        }

        // 以太网帧头解析
        auto* eth = reinterpret_cast<const ethernet_header*>(packet);
        if (!eth) {
            info.info  = "[SystemError]: parse ethernet header failed, ";
            info.info += parseIPPacketToHexString(buf, size);
            return info;
        }

        ss.clear();
        for (int i = 0; i < 6; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(eth->src_mac[i]);
            if (i < 5) ss << ":";
        }
        info.srcMAC = ss.str();

        ss.clear();
        for (int i = 0; i < 6; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(eth->dest_mac[i]);
            if (i < 5) ss << ":";
        }
        info.destMAC = ss.str();

        // IP 头解析
        const ip_header* ip = reinterpret_cast<const ip_header*>(packet + sizeof(ethernet_header));
        if (!ip) {
            info.info  = "[SystemError]: parse ip header failed, ";
            info.info += parseIPPacketToHexString(buf, size);
            return info;
        }

        ss.clear();
        uint8_t version = iph->ihl_and_version >> 4;  //取高四位作为版本号
        if (version == 4) {
            // 解析IPv4地址
            struct in_addr ip_addr;
            ip_addr.s_addr = iph->iph_sourceip;
            info.srcIP = inet_ntoa(ip_addr);
            ip_addr.s_addr = iph->iph_destip;
            info.destIP = inet_ntoa(ip_addr);

            // 计算 IP 头部的长度
            //uint8_t version = (ip->ihl_and_version >> 4) & 0xF;
            size_t ipHeaderLen = (ip->ihl_and_version & 0xF) * 4;

            // 获取协议类型 (TCP 或 UDP)
            size_t protocolHeaderLen = 0;
            if (ip->iph_protocol == IPPROTO_TCP) {
                // TCP 头的长度 (可能包括选项)
                auto len = sizeof(ethernet_header) + ipHeaderLen + sizeof(tcp_header);
                if (size < len) {
                    ss << "[TcpHeaderError]: capture length " << size << " less than tcp header min length " << len << ", ";
                    info.info = ss.str()+parseIPPacketToHexString(buf, size);
                    return info;
                }

                const tcp_header* tcp = reinterpret_cast<const tcp_header*>(packet + sizeof(ethernet_header) + ipHeaderLen);
                if (!tcp) {
                    info.info  = "[SystemError]: parse tcp header failed, ";
                    info.info += parseIPPacketToHexString(buf, size);
                    return info;
                }

                protocolHeaderLen = ((tcp->data_offset_and_reserved >> 4) & 0xF) * 4; // 提取 th_off 的值
                if (protocolHeaderLen  == 0) {
                    printf("tcp header error: header length is 0 and  data_offset_and_reserved %d \n", tcp->data_offset_and_reserved);
                    err = CapturePacketError::UdpHeaderLostError;
                    return info;
                }
            }
            else if (ip->iph_protocol == IPPROTO_UDP) {
                auto len = sizeof(ethernet_header) + ipHeaderLen + sizeof(udp_header);
                if (size < len) {
                    printf("udp header error: capture length:%d, udp header min length:%zu \n", size, len);
                    err = CapturePacketError::UdpHeaderLostError;
                    return info;
                }

                // UDP 头的长度是固定的
                protocolHeaderLen = sizeof(udp_header);
            }
            else {
                err = CapturePacketError::NoError;
                return info;
            }

            auto len = sizeof(ethernet_header) + ipHeaderLen + protocolHeaderLen;
            if (size < len) {
                printf("protocol header error: capture length:%d, protocol header min length:%zu \n", size, len);
                err = CapturePacketError::ProtocolHeaderLostError;
                return info;
            }
        } else if (version == 6) {
            // 解析IPv6地址
            struct ip6_header *iph6 = (struct ip6_header *)buffer;
            char ip6str[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, iph6->src_addr, ip6str, INET6_ADDRSTRLEN);
            info.srcIP = ip6str;
            inet_ntop(AF_INET6, iph6->dst_addr, ip6str, INET6_ADDRSTRLEN);
            info.destIP = ip6str;

            info.len = 0;
            info.info = parseIPPacketToHexString(buf, size);
        }

        return info;
    }

    static bool checkIpVersion(uint8_t ihl_and_version) {
        uint8_t version = ihl_and_version >> 4; // 假设 packet 指向 IP 头部的开始

        switch (version) {
        case 4:
        case 6:
            return true;
        default:
            break;
        }

        return false;
    }

    static bool isIpHeaderLengthValid(const ip_header* ipHeader, const size_t& packetSize) {
        uint8_t ihl = ipHeader->ihl_and_version & 0x0F;
        size_t headerLength = ihl * 4;

        // 检查头部长度是否至少为 20 字节
        if (ihl < 5) {
            return false;
        }

        // 确保头部长度不超过数据包的总长度
        if (headerLength > packetSize) {
            return false;
        }

        return true;
    }

    static bool isIpTotalLengthValid(const ip_header* ipHeader, const size_t& packetSize) {
        uint16_t totalLength = ntohs(ipHeader->iph_len);

        // 检查总长度是否至少为 IP 头部的最小长度
        if (totalLength < 20) {
            return false;
        }

        // 检查总长度是否不超过接收到的数据包的总长度
        if (totalLength > packetSize) {
            return false;
        }

        return true;
    }

    static bool isIpTtlValid(const ip_header* ipHeader) {
        // 检查 TTL 是否为 0, 1
        if (ipHeader->iph_ttl < 2) {
            return false; // TTL 为 0，表示数据包已经在网络中传输太久 -可选  1 警告低 TTL 值
        }

        return true;
    }

    static uint16_t computeIpChecksum(const ip_header* ipHeader) {
        uint32_t sum = 0;
        const uint16_t* ptr = reinterpret_cast<const uint16_t*>(ipHeader);

        // 将头部的每个 16 位字相加
        for (int i = 0; i < sizeof(ip_header) / 2; ++i) {
            if (i != 5) { // 跳过原校验和字段
                sum += ntohs(ptr[i]);
            }
        }

        // 将进位加到最低位
        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        // 取补码
        return ~sum;
    }

    static bool isIpAddressValid(uint32_t ip) {
        // 转换为主机字节序
        ip = ntohl(ip);

        // 检查特殊地址
        if (ip == 0 || ip == 0xFFFFFFFF) { // 0.0.0.0 和 255.255.255.255
            return false;
        }

        // 检查保留地址
        // 示例：127.0.0.0/8 (本地回环地址)
        if ((ip & 0xFF000000) == 0x7F000000) {
            return false;
        }

        // 添加更多地址范围的检查...
        /*0.0.0.0：

        在客户端，这个地址通常用于表示“任意地址”或“所有接口”，例如，在绑定套接字时用作源地址。
        在服务器端，它通常表示监听所有可用网络接口。
        255.255.255.255：

        这是一个有限广播地址，用于在本地网络上发送广播消息。
        它通常不用于源地址，但可用于目的地址以在本地网络上发送广播。
        回环地址(127.0.0.1 / 8)：

        这是用于本地通信的地址。向这个地址发送的数据不会离开主机，而是直接回环。
        它通常用于测试和本地通信。*/
        return true;
    }

    IpError checkIpHeader(const ip_header* ipHeader, const size_t& packetSize) {
        if (!checkIpVersion(ipHeader->ihl_and_version))
            return IpError::VersionUnknown;

        if (!isIpHeaderLengthValid(ipHeader, packetSize))
            return IpError::HeaderLengthError;

        if (!isIpTotalLengthValid(ipHeader, packetSize))
            return IpError::TotalLengthError;

        if (!isIpTtlValid(ipHeader))
            return IpError::TTLInvalidOrWarn;

        /*校验和为 0 的情况
            某些协议不要求校验和：例如，当使用 IPv4 的某些协议（如 UDP）时，校验和可能是可选的。在这种情况下，校验和字段可能被设置为 0。
            特殊网络环境：在一些特殊的网络环境或配置中，校验和可能被设置为 0。例如，某些类型的虚拟化软件或特殊的网络配置可能不使用标准的校验和。
        */ 
        uint16_t headerChecksum = ntohs(ipHeader->iph_chksum); // 将网络字节序转换为主机字节序
        if (headerChecksum == 0)
            return IpError::NoError;

        auto checkSum = computeIpChecksum(ipHeader);
        if (checkSum != headerChecksum) {
            printf("compute ip checksum error: compute checksum:%d, header checksum:%d\n", checkSum, headerChecksum);
            return IpError::InvalidChecksum;
        }

        return IpError::NoError;
    }
    
    // 函数：校验 TCP 端口号
    static TcpError checkTcpPorts(const tcp_header* tcpHeader) {
        // 确保端口号是大端序转换为主机字节序后的非零值
        uint16_t sourcePort = ntohs(tcpHeader->th_sport);
        uint16_t destinationPort = ntohs(tcpHeader->th_dport);

        // 检查源端口号是否为非零值
        if (sourcePort == 0) {
            return TcpError::InvalidSourcePort;
        }

        // 检查目标端口号是否为非零值
        if (destinationPort == 0) {
            return TcpError::InvalidDestinationPort;
        }

        // 端口号有效
        return TcpError::NoError;
    }

    // 函数：校验 TCP 头部长度
    static TcpError checkTcpHeaderLength(const tcp_header* tcpHeader) {
        // 提取数据偏移值，它位于头部的第一个字节的高四位
        uint8_t dataOffset = tcpHeader->data_offset_and_reserved >> 4;

        // 校验头部长度是否至少为 5（即 TCP 头部最小长度 20 字节）
        if (dataOffset < 5) {
            printf("check tcp header length error: %d less than 5\n", dataOffset);
            return TcpError::HeaderLengthError;
        }

        // 校验头部长度是否合理，最大值为 15（即包含最多可选字段的 60 字节）
        if (dataOffset > 15) {
            printf("check tcp header length error: %d greater than 15\n", dataOffset);
            return TcpError::HeaderLengthError;
        }

        // 头部长度有效
        return TcpError::NoError;
    }

    static uint16_t computeChecksum(const uint16_t* buffer, int size) {
        uint32_t sum = 0;
        while (size > 1) {
            sum += *buffer++;
            size -= 2;
        }
        if (size > 0) {
            sum += *reinterpret_cast<const uint8_t*>(buffer);
        }
        while (sum >> 16) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }
        return ~sum;
    }

    // 构建 TCP 伪头部
    struct pseudo_header {
        uint32_t source_address;
        uint32_t dest_address;
        uint8_t placeholder;
        uint8_t protocol;
        uint16_t tcp_length;
    } pseudoHdr;

    static uint16_t calculateTcpChecksum(const ip_header* ipHeader, int ipHeaderLen, const tcp_header* tcpHeader) {

        pseudoHdr.source_address = ipHeader->iph_sourceip;
        pseudoHdr.dest_address = ipHeader->iph_destip;
        pseudoHdr.placeholder = 0;
        pseudoHdr.protocol = IPPROTO_TCP;
        pseudoHdr.tcp_length = htons(ntohs(ipHeader->iph_len) - ipHeaderLen);

        // 计算伪头部和 TCP 头部的校验和
        int psize = sizeof(pseudo_header) + ntohs(pseudoHdr.tcp_length);
        std::vector<uint8_t> buff(psize);
        memcpy(buff.data(), &pseudoHdr, sizeof(pseudo_header));
        memcpy(buff.data() + sizeof(pseudo_header), tcpHeader, ntohs(pseudoHdr.tcp_length));

        return computeChecksum(reinterpret_cast<const uint16_t*>(buff.data()), psize);
    }

    TcpError checkTcpHeader(const unsigned char* packet) {
        // 提取 IP 头和 TCP 头
        const ip_header* ipHeader = reinterpret_cast<const ip_header*>(packet + sizeof(ethernet_header));
        int ipHeaderLen = (ipHeader->ihl_and_version & 0x0F) * 4;
        const tcp_header* tcpHeader = reinterpret_cast<const tcp_header*>(packet + sizeof(ethernet_header) + ipHeaderLen);

        auto portCheckError = checkTcpPorts(tcpHeader);
        if (portCheckError != TcpError::NoError)
            return portCheckError;

        auto headerLengthError = checkTcpHeaderLength(tcpHeader);
        if (headerLengthError != TcpError::NoError) 
            return headerLengthError;

        uint16_t headerChecksum = ntohs(tcpHeader->th_sum); // 将网络字节序转换为主机字节序
        auto checkSum = calculateTcpChecksum(ipHeader, ipHeaderLen, tcpHeader);
        if (checkSum != headerChecksum) {
            printf("compute tcp checksum error: compute checksum:%d, header checksum:%d\n", checkSum, headerChecksum);
            return TcpError::InvalidChecksum;
        }
        return TcpError::NoError;
    }
#if 0

    // 功能：将点分十进制格式的 IP 地址转换为字节
    std::vector<uint8_t> ipToBytes(const std::string& ip) {
        std::vector<uint8_t> bytes;
        std::istringstream ipStream(ip);
        std::string byteString;
        while (std::getline(ipStream, byteString, '.')) {
            bytes.push_back(static_cast<uint8_t>(std::stoi(byteString)));
        }
        return bytes;
    }

    // 功能：将冒号分隔格式的 MAC 地址转换为字节
    std::vector<uint8_t> macToBytes(const std::string& mac) {
        std::vector<uint8_t> bytes;
        std::istringstream macStream(mac);
        std::string byteString;
        while (std::getline(macStream, byteString, ':')) {
            bytes.push_back(static_cast<uint8_t>(std::stoi(byteString, nullptr, 16)));
        }
        return bytes;
    }

    // 将 unsigned int 分解为字节并存入 vector<uint8_t>
    std::vector<uint8_t> uintToBytes(unsigned int value) {
        std::vector<uint8_t> bytes;
        bytes.push_back((value >> 24) & 0xFF); // 提取最高有效字节
        bytes.push_back((value >> 16) & 0xFF); // 提取次高有效字节
        bytes.push_back((value >> 8) & 0xFF);  // 提取次低有效字节
        bytes.push_back(value & 0xFF);         // 提取最低有效字节
        return bytes;
    }

    // 辅助函数：计算校验和
    uint16_t calculateChecksum(std::vector<uint8_t>& buffer, size_t start, size_t len) {
        uint32_t sum = 0;
        for (size_t i = start; i < start + len; i += 2) {
            sum += (buffer[i] << 8) + (i + 1 < start + len ? buffer[i + 1] : 0);
        }
        sum = (sum >> 16) + (sum & 0xFFFF);
        return static_cast<uint16_t>(~sum);
    }

    // 构造 UDP 数据包
    std::vector<uint8_t> constructUdpPacket(const unsigned char* raw_packet,
        uint16_t dstPort, uint16_t srcPort,
        const std::vector<uint8_t>& udpData) {

        std::vector<uint8_t> packet;

        // 以太网头部
        packet.insert(packet.end(), raw_packet + 6, raw_packet + 11);
        packet.insert(packet.end(), raw_packet, raw_packet + 6);
        packet.push_back(0x08); packet.push_back(0x00);// IPv4 Ethertype

        // IPv4 头部
        packet.push_back(0x45); // 版本和头部长度
        packet.push_back(0x00); // 服务类型
        uint16_t totalLength = htons(20 + 8 + udpData.size()); // 总长度
        packet.push_back(totalLength >> 8);
        packet.push_back(totalLength & 0xFF);
        packet.push_back(0x00); packet.push_back(0x00); // 标识
        packet.push_back(0x40); packet.push_back(0x00); // 标志和片偏移
        packet.push_back(0x40); // TTL
        packet.push_back(0x11); // 协议 (UDP)
        packet.push_back(0x00); packet.push_back(0x00); // 头部校验和，稍后计算

        // 源 IP 和目标 IP
        const ip_header* ipHeader = reinterpret_cast<const ip_header*>(raw_packet + sizeof(ethernet_header));
        auto srcIpBytes = uintToBytes(ipHeader->iph_destip);
        auto dstIpBytes = uintToBytes(ipHeader->iph_sourceip);
        packet.insert(packet.end(), srcIpBytes.begin(), srcIpBytes.end());
        packet.insert(packet.end(), dstIpBytes.begin(), dstIpBytes.end());

        // 计算 IP 头部校验和
        uint16_t ipChecksum = calculateChecksum(packet, 14, 20);
        packet[24] = ipChecksum >> 8;
        packet[25] = ipChecksum & 0xFF;

        // UDP 头部
        packet.push_back(srcPort >> 8); packet.push_back(srcPort & 0xFF);
        packet.push_back(dstPort >> 8); packet.push_back(dstPort & 0xFF);
        uint16_t udpLength = htons(8 + udpData.size());
        packet.push_back(udpLength >> 8); packet.push_back(udpLength & 0xFF);
        packet.push_back(0x00); packet.push_back(0x00); // 校验和占位符

        // UDP 数据
        packet.insert(packet.end(), udpData.begin(), udpData.end());

        // 计算 UDP 校验和（包括伪头部）
        std::vector<uint8_t> pseudoHeader;
        pseudoHeader.insert(pseudoHeader.end(), srcIpBytes.begin(), srcIpBytes.end());
        pseudoHeader.insert(pseudoHeader.end(), dstIpBytes.begin(), dstIpBytes.end());
        pseudoHeader.push_back(0x00); pseudoHeader.push_back(0x11);
        pseudoHeader.push_back(udpLength >> 8); pseudoHeader.push_back(udpLength & 0xFF);

        std::vector<uint8_t> udpSegment;
        udpSegment.insert(udpSegment.end(), pseudoHeader.begin(), pseudoHeader.end());
        udpSegment.insert(udpSegment.end(), packet.begin() + 34, packet.end()); // 34 是 IP 头部的起始位置

        uint16_t udpChecksum = calculateChecksum(udpSegment, 0, udpSegment.size());
        packet[40] = udpChecksum >> 8; // 40 是 UDP 头部校验和的位置
        packet[41] = udpChecksum & 0xFF;

        return packet;
    }
#endif
}
