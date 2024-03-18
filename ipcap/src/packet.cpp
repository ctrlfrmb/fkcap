// ipcap.cpp: 定义应用程序的入口点。
//
#include <iomanip>
#include <sstream>
#include <atomic>
#include <chrono>
#ifdef _WIN32
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x6000
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#endif // _WIN32

#include "def.h"
#include "packet.h"

namespace figkey {

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

    std::string parsePayloadToHexString(const std::vector<uint8_t>& data) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        for (const auto& d : data)
            ss << std::setw(2) << static_cast<unsigned>(d) << " ";

        std::string result = ss.str();
        // Remove the last space
        if (!result.empty())
            result.pop_back();

        return result;
    }

    static std::string convertMacToString(const uint8_t mac[6]){
        char buf[17];
        sprintf(buf, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return std::string(buf);
    }

    static bool parseEthernet(const unsigned char* packet, const uint32_t& size, PacketInfo &info) {
        // 以太网帧头解析
        auto* eth = reinterpret_cast<const ethernet_header*>(packet);
        if (!eth) {
            info.data = "[SystemError]: parse ethernet header failed";
            info.err = PACKET_SYSTEM_ERROR;
            return false;
        }

        // 假设以太网头部使用 Ethernet II 格式，则下一个协议类型字段为 2 字节
        uint16_t type = ntohs(eth->type);
        if (type == 0x0800) {
            if (size <= ETHERNET_IPV4_HEADER_MIN) {
                char buf[256];
                sprintf(buf, "[Ipv4HeadError]: IPv4 packet loss, the current packet length %u is less than the required minimum packet length %d", size, ETHERNET_IPV4_HEADER_MIN);
                info.data = buf;
                info.err = PACKET_IPV4_HEADER_LOST_ERROR;
                return false;
            }
            // IPv4
            info.protocolType = 4;
        } else if (type == 0x86DD) {
            if (size <= ETHERNET_IPV6_HEADER_MIN) {
                char buf[256];
                sprintf(buf, "[Ipv6HeadError]: IPv6 packet loss, the current packet length %u is less than the required minimum packet length %d", size, ETHERNET_IPV6_HEADER_MIN);
                info.data = buf;
                info.err = PACKET_IPV6_HEADER_LOST_ERROR;
                return false;
            }
            // IPv6
            info.protocolType = 6;
        } else {
            // 不是 IP 数据包或者我们暂时不处理的类型
            info.data = "[EthernetError]: Ethernet type unknown";
            info.err = PACKET_ETHERNET_TYPE_UNKNOWN;
            return false;
        }

        info.srcMAC = convertMacToString(eth->src_mac);
        info.destMAC = convertMacToString(eth->dest_mac);
        return true;
    }

    static std::string convertIpToString(uint32_t ip) {
        struct in_addr ip_addr;
        ip_addr.s_addr = ip;
        return inet_ntoa(ip_addr);
    }

    static size_t parseIPv4(const unsigned char* packet, PacketInfo &info) {
        // IP 头解析
        const ip_header* iph = reinterpret_cast<const ip_header*>(packet);
        if (!iph) {
            info.data  = "[SystemError]: parse ipv4 header failed";
            info.err = PACKET_SYSTEM_ERROR;
            return 0;
        }

        info.srcIP = convertIpToString(iph->iph_sourceip);
        info.destIP = convertIpToString(iph->iph_destip);
        info.protocolType = iph->iph_protocol;
        info.payloadLength = ntohs(iph->iph_len) - ((iph->ihl_and_version & 0xF) * 4);  // 注意网络到主机字节序的转换
        // 计算 IP 头部的长度
        //uint8_t version = (ip->ihl_and_version >> 4) & 0xF;
        size_t len = sizeof(ethernet_header)+(iph->ihl_and_version & 0xF) * 4;
        return len;
    }

    static size_t parseIPv6(const unsigned char* packet, PacketInfo &info) {
        const ip6_header* iph = reinterpret_cast<const ip6_header*>(packet);
        if (!iph) {
            info.data  = "[SystemError]: parse ipv6 header failed";
            info.err = PACKET_SYSTEM_ERROR;
            return 0;
        }

        char straddr[INET6_ADDRSTRLEN];
        inet_ntop(AF_INET6, (void *)iph->src_addr, straddr, INET6_ADDRSTRLEN);
        info.srcIP = std::string(straddr);

        inet_ntop(AF_INET6, (void *)iph->dst_addr, straddr, INET6_ADDRSTRLEN);
        info.destIP = std::string(straddr);

        info.protocolType = iph->next_header;
        info.payloadLength = iph->payload_len;
        size_t len = sizeof(ethernet_header)+sizeof(ip6_header); //固定40，扩展需令计算
        return len;
    }

    static std::string parsePayloadToHexString(const unsigned char* data, size_t length) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');

        for (size_t i = 0; i < length; ++i) {
            ss << std::setw(2) << static_cast<unsigned>(data[i]) << " ";
        }

        return ss.str();
    }

    static size_t parseTCP(const unsigned char* packet, const uint32_t& size, PacketInfo &info) {
        info.protocolType = PROTOCOL_TYPE_TCP;
        // TCP 头的长度 (可能包括选项)
        auto len = sizeof(tcp_header);
        if (size < len) {
            info.data = "[TcpHeaderError]: the packet length is less than the tcp header minimum value " ;
            info.err = PACKET_TCP_HEADER_LOST_ERROR;
            return 0;
        }

        const tcp_header* tcph = reinterpret_cast<const tcp_header*>(packet);
        if (!tcph) {
            info.data  = "[SystemError]: parse tcp header failed";
            info.err = PACKET_SYSTEM_ERROR;
            return 0;
        }

        auto headerLen = ((tcph->data_offset_and_reserved >> 4) & 0xF) * 4; // 提取 th_off 的值
        if (headerLen  == 0) {
            info.data = "[TcpHeaderError]: data_offset_and_reserved is 0";
            info.err = PACKET_TCP_HEADER_OFFSET_ERROR;
            return 0;
        }

        info.srcPort = ntohs(tcph->th_sport);
        info.destPort = ntohs(tcph->th_dport);
        info.payloadLength -= headerLen;
        auto dataLen = size-len;
        if (info.payloadLength > dataLen) {
            info.data = "[TcpPayloadError]: tcp payload data loss, " ;
            info.data += parsePayloadToHexString(packet+len, dataLen);
            info.err = PACKET_TCP_PAYLOAD_LOST_ERROR;
            return 0;
        }
        return len;
    }

    static size_t parseUDP(const unsigned char* packet, const uint32_t& size, PacketInfo &info) {
        info.protocolType = PROTOCOL_TYPE_UDP;
        auto len = sizeof(udp_header);
        if (size < len) {
            info.data = "[UdpHeaderError]: the packet length is less than the udp header minimum value " ;
            info.err = PACKET_TCP_HEADER_LOST_ERROR;
            return 0;
        }
        struct udp_header* udph = (struct udp_header*)(packet);
        info.srcPort = ntohs(udph->uh_sport);
        info.destPort = ntohs(udph->uh_dport);
        // 这里可以根据需要将 UDP 报文的其他字段也解析出来
        //info.payloadLength -= len;
        info.payloadLength = ntohs(udph->uh_len)-len;
        auto dataLen = size-len;
        if (info.payloadLength > dataLen) {
            info.data = "[UdpPayloadError]: udp payload data loss, " ;
            info.data += parsePayloadToHexString(packet+len, dataLen);
            info.err = PACKET_UDP_PAYLOAD_LOST_ERROR;
            return 0;
        }
        return len;
    }

    PacketInfo parseIpPacket(const unsigned char* packet, const uint32_t& size) {
        PacketInfo info;
        info.protocolType = PROTOCOL_TYPE_DEFAULT;
        info.err = PACKET_NO_ERROR;

        if (size < ETHERNET_IP_UDP_HEADER_MIN) {
            if (size > 0)
                info.data = "[SnapLengthError]: capture length less than udp header min length 42";
            else
                info.data = "[SnapLengthError]: capture length is 0";
            info.err = PACKET_SYSTEM_ERROR;
            return info;
        }

        if (!parseEthernet(packet, size, info))
            return info;

        size_t offset = sizeof(ethernet_header);
        if (4 == info.protocolType)
            offset = parseIPv4(packet+offset,info);
        else
            offset = parseIPv6(packet+offset,info);
        if (0 == offset)
            return info;

        size_t headerLen{0};
        switch (info.protocolType) {
            case IPPROTO_TCP:
                headerLen = parseTCP(packet+offset, size - offset, info);
                break;
            case IPPROTO_UDP:
                headerLen = parseUDP(packet+offset, size - offset, info);
                break;
            default:
                info.data = "[NoError]: current protocol is not tcp or udp";
                return info;
        }
        if (0 == headerLen)
            return info;

        offset += headerLen;
        info.index = offset; //temporary storage offset
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
        for (size_t i = 0; i < sizeof(ip_header) / 2; ++i) {
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

//    static bool isIpAddressValid(uint32_t ip) {
//        // 转换为主机字节序
//        ip = ntohl(ip);

//        // 检查特殊地址
//        if (ip == 0 || ip == 0xFFFFFFFF) { // 0.0.0.0 和 255.255.255.255
//            return false;
//        }

//        // 检查保留地址
//        // 示例：127.0.0.0/8 (本地回环地址)
//        if ((ip & 0xFF000000) == 0x7F000000) {
//            return false;
//        }

//        // 添加更多地址范围的检查...
//        /*0.0.0.0：

//        在客户端，这个地址通常用于表示“任意地址”或“所有接口”，例如，在绑定套接字时用作源地址。
//        在服务器端，它通常表示监听所有可用网络接口。
//        255.255.255.255：

//        这是一个有限广播地址，用于在本地网络上发送广播消息。
//        它通常不用于源地址，但可用于目的地址以在本地网络上发送广播。
//        回环地址(127.0.0.1 / 8)：

//        这是用于本地通信的地址。向这个地址发送的数据不会离开主机，而是直接回环。
//        它通常用于测试和本地通信。*/
//        return true;
//    }

    uint8_t checkIpHeader(const ip_header* ipHeader, const size_t& packetSize) {
        if (!checkIpVersion(ipHeader->ihl_and_version))
            return PACKET_IP_VERSION_UNKNOWN;

        if (!isIpHeaderLengthValid(ipHeader, packetSize))
            return PACKET_IP_HEADER_LENGTH_ERROR;

        if (!isIpTotalLengthValid(ipHeader, packetSize))
            return PACKET_IP_TOTAL_LENGTH_ERROR;

        if (!isIpTtlValid(ipHeader))
            return PACKET_IP_TTL_INVALID_OR_WARN;

        /*校验和为 0 的情况
            某些协议不要求校验和：例如，当使用 IPv4 的某些协议（如 UDP）时，校验和可能是可选的。在这种情况下，校验和字段可能被设置为 0。
            特殊网络环境：在一些特殊的网络环境或配置中，校验和可能被设置为 0。例如，某些类型的虚拟化软件或特殊的网络配置可能不使用标准的校验和。
        */ 
        uint16_t headerChecksum = ntohs(ipHeader->iph_chksum); // 将网络字节序转换为主机字节序
        if (headerChecksum == 0)
            return PACKET_NO_ERROR;

        auto checkSum = computeIpChecksum(ipHeader);
        if (checkSum != headerChecksum) {
            printf("compute ip checksum error: compute checksum:%d, header checksum:%d\n", checkSum, headerChecksum);
            return PACKET_IP_INVALID_CHECKSUM;
        }

        return PACKET_NO_ERROR;
    }
    
    // 函数：校验 TCP 端口号
    static uint8_t checkTcpPorts(const tcp_header* tcpHeader) {
        // 确保端口号是大端序转换为主机字节序后的非零值
        uint16_t sourcePort = ntohs(tcpHeader->th_sport);
        uint16_t destinationPort = ntohs(tcpHeader->th_dport);

        // 检查源端口号是否为非零值
        if (sourcePort == 0) {
            return PACKET_TCP_INVALID_SOURCE_PORT;
        }

        // 检查目标端口号是否为非零值
        if (destinationPort == 0) {
            return PACKET_TCP_INVALID_DESTINATION_PORT;
        }

        // 端口号有效
        return PACKET_NO_ERROR;
    }

    // 函数：校验 TCP 头部长度
    static uint8_t checkTcpHeaderLength(const tcp_header* tcpHeader) {
        // 提取数据偏移值，它位于头部的第一个字节的高四位
        uint8_t dataOffset = tcpHeader->data_offset_and_reserved >> 4;

        // 校验头部长度是否至少为 5（即 TCP 头部最小长度 20 字节）
        if (dataOffset < 5) {
            printf("check tcp header length error: %d less than 5\n", dataOffset);
            return PACKET_TCP_HEADER_LENGTH_ERROR;
        }

        // 校验头部长度是否合理，最大值为 15（即包含最多可选字段的 60 字节）
        if (dataOffset > 15) {
            printf("check tcp header length error: %d greater than 15\n", dataOffset);
            return PACKET_TCP_HEADER_LENGTH_ERROR;
        }

        // 头部长度有效
        return PACKET_NO_ERROR;
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

    uint8_t checkTcpHeader(const unsigned char* packet) {
        // 提取 IP 头和 TCP 头
        const ip_header* ipHeader = reinterpret_cast<const ip_header*>(packet + sizeof(ethernet_header));
        int ipHeaderLen = (ipHeader->ihl_and_version & 0x0F) * 4;
        const tcp_header* tcpHeader = reinterpret_cast<const tcp_header*>(packet + sizeof(ethernet_header) + ipHeaderLen);

        auto portCheckError = checkTcpPorts(tcpHeader);
        if (portCheckError != PACKET_NO_ERROR)
            return portCheckError;

        auto headerLengthError = checkTcpHeaderLength(tcpHeader);
        if (headerLengthError != PACKET_NO_ERROR)
            return headerLengthError;

        uint16_t headerChecksum = ntohs(tcpHeader->th_sum); // 将网络字节序转换为主机字节序
        auto checkSum = calculateTcpChecksum(ipHeader, ipHeaderLen, tcpHeader);
        if (checkSum != headerChecksum) {
            printf("compute tcp checksum error: compute checksum:%d, header checksum:%d\n", checkSum, headerChecksum);
            return PACKET_TCP_INVALID_CHECKSUM;
        }
        return PACKET_NO_ERROR;
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
