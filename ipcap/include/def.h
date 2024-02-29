/**
 * @file    def.h
 * @ingroup figkey
 * @brief   
 * @author  leiwei
 * @date    2023.12.19
 * Copyright (c) figkey 2023-2033
 */

#pragma once

#ifndef FIGKEY_PCAP_DEF_HPP
#define FIGKEY_PCAP_DEF_HPP

#include <cstdint>
#include <vector>
#include <string>

#define CAPTURE_SNAP_LENGTH 65536
#define CAPTURE_PROMISC 1
#define CONFIG_ROOT_NODE_NAME "ipcap"
#define CONFIG_SAVE_LOG_NODE "CaptureType"
#define CONFIG_CAPTURE_TYPE_NODE "SaveLog"
#define CONFIG_RUN_ASYNC_NODE "RunAsync"
#define CONFIG_FILTER_PROTOCOL_NODE "FilterProtocol"
#define CONFIG_FILTER_MAC_NODE "FilterMac"
#define CONFIG_FILTER_IP_NODE "FilterIp"
#define CONFIG_FILTER_PORT_NODE "FilterPort"
#define PACKET_LOGGER_ERROR "error"
#define PACKET_LOGGER_WARN "warn"

namespace figkey {

    enum class CapturePacketError : uint8_t {
        NoError = 0,
        SystemError,                // 系统环境错误
        SnapLengthError,            // 捕获异常
        IpHeaderLostError,          // IP 包丢失
        TcpHeaderLostError,         // TCP 包丢失
        TcpHeaderOffsetError,       // TCP 头偏移量错误
        UdpHeaderLostError,         // UDP 包丢失
        ProtocolHeaderLostError     // 协议头丢失
        // ... 其他错误
    };

    enum class IpError : uint8_t {
        NoError = 0,
        VersionUnknown,             // IP 版本未知
        HeaderLengthError,          // 头部长度错误
        TotalLengthError,           // IP 包不完整
        TTLInvalidOrWarn,           // TTL 为0或 1
        InvalidChecksum             // 无效校验和
    };

    enum class TcpError : uint8_t {
        NoError = 0,
        InvalidSourcePort,
        InvalidDestinationPort,
        HeaderLengthError,          // 头部长度错误
        InvalidChecksum,            // 无效校验和
        Retransmission,             // 重传
        OutOfOrder,                 // 乱序
        LostSegment,                // 丢失分段
        DuplicateAck,               // 重复确认
        WindowFull,                 // 窗口已满
        ZeroWindow                  // 零窗口（流量控制）
        // ... 其他错误
    };

    enum class UdpError : uint8_t {
        NoError = 0,
        HeaderLengthError,          // 头部长度错误
        InvalidChecksum,            // 无效校验和
        PortUnreachable             // 端口不可达
        // ... 其他错误
    };

    // Enumeration for capture packet type
    enum class ProtocolType : uint8_t {
        DEFAULT = 0x0,         //默认TCP/UDP数据
        TCP=0x02,
        UDP=0x04,
        DOIP=0x08,              //监控DOIP数据
        UDS=0x10                //监控UDS数据
    };

    // Structure for load capture config information
    struct CaptureConfigInfo {
        ProtocolType type{ ProtocolType::DEFAULT };
        bool save{ true };
        bool async{ false };
        std::string filter{ "" };
    };

    // Structure for storing ip address information
    struct IpAddressInfo {
        std::string ip;
        std::string netmask;
    };

    // Structure for storing network interface information
    struct NetworkInfo {
        std::string name;
        std::string description;
        std::vector<IpAddressInfo> address;
    };

    // Structure for storing packet logger information
    struct PacketLoggerInfo {
        std::string index{""};
        std::string timestamp;
        std::string address;
    };

    // 以太网帧头定义
    struct ethernet_header {
        uint8_t dest_mac[6];  // 目标 MAC 地址
        uint8_t src_mac[6];   // 源 MAC 地址
        uint16_t type;        // 协议类型
    };

    // IP 头定义
    struct ip_header {
        uint8_t  ihl_and_version;     // 包含 iph_ihl : 4, iph_ver : 4;防止大小端内存字节序错乱
        uint8_t  iph_tos;             // 服务类型
        uint16_t iph_len;             // 总长度
        uint16_t iph_ident;           // 标识
        uint16_t iph_offset;          // 标志和片偏移
        uint8_t  iph_ttl;             // 生存时间
        uint8_t  iph_protocol;        // 协议
        uint16_t iph_chksum;          // 头部校验和
        uint32_t iph_sourceip;        // 源地址
        uint32_t iph_destip;          // 目的地址
    };

    struct ip6_header {
        unsigned int
               version : 4,
               traffic_class : 8,
               flow_label : 20;
        uint16_t payload_len;
        uint8_t next_header;
        uint8_t hop_limit;
        uint8_t src_addr[16];
        uint8_t dst_addr[16];
    };

    // TCP 头定义
    struct tcp_header {
        uint16_t th_sport;     // 源端口
        uint16_t th_dport;     // 目的端口
        uint32_t th_seq;       // 序列号
        uint32_t th_ack;       // 确认号
        uint8_t  data_offset_and_reserved; // 包含 th_off : 4,// 数据偏移 th_x2 : 4; // （保留）防止大小端内存字节序错乱
        uint8_t  th_flags;     // 标志位
        uint16_t th_win;       // 窗口大小
        uint16_t th_sum;       // 校验和
        uint16_t th_urp;       // 紧急指针
    };

    // UDP 头定义
    struct udp_header {
        uint16_t uh_sport;     // 源端口
        uint16_t uh_dport;     // 目的端口
        uint16_t uh_len;       // 长度
        uint16_t uh_sum;       // 校验和
    };

    // DOIP 报头 NACK 代码
    enum class DoIPHeaderNackCode : uint8_t {
        None = 0,
        IncorrectPatternFormat,
        UnknownPayloadType,
        MessageTooLarge,
        InvalidPayloadLength,
        OutOfMemory
    };

    // 定义 DOIP 载荷类型
    enum class DoIPPayloadType : uint16_t {
        GenericHeaderNack = 0x0000,
        VehicleIdentificationRequest = 0x0001,
        VehicleIdentificationRequestWithEID = 0x0002,
        VehicleIdentificationRequestWithVIN = 0x0003,
        VehicleIdentificationResponseOrAnnouncement = 0x0004,
        RoutingActivationRequest = 0x0005,
        RoutingActivationResponse = 0x0006,
        AliveCheckRequest = 0x0007,
        AliveCheckResponse = 0x0008,
        DoIPEntityStatusRequest = 0x4001,
        DoIPEntityStatusResponse = 0x4002,
        DiagnosticPowerModeRequest = 0x4003,
        DiagnosticPowerModeResponse = 0x4004,
        DiagnosticMessage = 0x8001,
        DiagnosticPositiveAck = 0x8002,
        DiagnosticNegativeAck = 0x8003
    };

    enum PACKET_ERROR : uint8_t {
        PACKET_NO_ERROR = 0,
        PACKET_SYSTEM_ERROR,                  // 系统环境错误
        PACKET_SNAP_LENGTH_ERROR,             // 捕获异常
        PACKET_ETHERNET_TYPE_UNKNOWN,         // 以太网类型未知
        PACKET_IPV4_HEADER_LOST_ERROR,        // IPv4 包丢失
        PACKET_IPV6_HEADER_LOST_ERROR,        // IPv6 包丢失
        PACKET_TCP_HEADER_LOST_ERROR,         // TCP 包丢失
        PACKET_TCP_HEADER_OFFSET_ERROR,       // TCP 头偏移量错误
        PACKET_UDP_HEADER_LOST_ERROR,         // UDP 包丢失
        PACKET_TCP_PAYLOAD_LOST_ERROR,        // TCP 数据缺失
        PACKET_UDP_PAYLOAD_LOST_ERROR,        // UDP 数据缺失

        PACKET_IP_VERSION_UNKNOWN,            // IP 版本未知
        PACKET_IP_HEADER_LENGTH_ERROR,        // 头部长度错误
        PACKET_IP_TOTAL_LENGTH_ERROR,         // IP 包不完整
        PACKET_IP_TTL_INVALID_OR_WARN,        // TTL 为0或 1
        PACKET_IP_INVALID_CHECKSUM,           // 无效校验和

        PACKET_TCP_INVALID_SOURCE_PORT,
        PACKET_TCP_INVALID_DESTINATION_PORT,
        PACKET_TCP_HEADER_LENGTH_ERROR,       // 头部长度错误
        PACKET_TCP_INVALID_CHECKSUM,          // 无效校验和
        PACKET_TCP_RETRANSMISSION,            // 重传
        PACKET_TCP_OUT_OF_ORDER,              // 乱序
        PACKET_TCP_LOST_SEGMENT,              // 丢失分段
        PACKET_TCP_DUPLICATE_ACK,             // 重复确认
        PACKET_TCP_WINDOW_FULL,               // 窗口已满
        PACKET_TCP_ZERO_WINDOW,               // 零窗口（流量控制）

        PACKET_UDP_HEADER_LENGTH_ERROR,       // 头部长度错误
        PACKET_UDP_INVALID_CHECKSUM,          // 无效校验和
        PACKET_UDP_PORT_UNREACHABLE           // 端口不可达
        // ... 其他错误
    };

    enum PACKET_TYPE : uint8_t {
        PROTOCOL_TYPE_DEFAULT,
        PROTOCOL_TYPE_IPV4,
        PROTOCOL_TYPE_IPV6,
        PROTOCOL_TYPE_TCP,
        PROTOCOL_TYPE_UDP,
        PROTOCOL_TYPE_DOIP,
        PROTOCOL_TYPE_UDS
    };

    struct PacketInfo
    {
        uint64_t index{0};               // 索引
        std::string timestamp;           // 时间戳
        uint8_t err;                     // 错误码
        std::string srcIP{""};           // 源IP
        std::string destIP{""};          // 目标IP
        std::string srcMAC;              // 源MAC
        std::string destMAC;             // 目标MAC
        uint16_t srcPort;                // 源端口
        uint16_t destPort;               // 目标端口
        uint16_t payloadLength;          // 负载长度
        uint8_t protocolType;            // 协议类型，使用枚举类表示
        std::string data;                // 信息
    };

}  // namespace figkey

#endif // !FIGKEY_PCAP_DEF_HPP
