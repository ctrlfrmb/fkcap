#ifndef DOIPGENERICHEADERHANDLER_H
#define DOIPGENERICHEADERHANDLER_H

#include <QByteArray>
#include <QtEndian>

#define DOIP_GENERIC_HEADER_LENGTH 8
#define DOIP_PROTOCOL_VERSION_MAX 0xFFU

#define DOIP_GENERIC_DOIP_NACK 0x0000
#define DOIP_VEHICLE_IDENTIFICATION_REQUEST 0x0001
#define DOIP_VEHICLE_IDENTIFICATION_REQUEST_WITH_EID 0x0002
#define DOIP_VEHICLE_IDENTIFICATION_REQUEST_WITH_VIN 0x0003
#define DOIP_VEHICLE_ANNOUNCEMENT 0x0004
#define DOIP_ROUTING_ACTIVATION_REQUEST 0x0005
#define DOIP_ROUTING_ACTIVATION_RESPONSE 0x0006
#define DOIP_ALIVE_CHECK_REQUEST 0x0007
#define DOIP_ALIVE_CHECK_RESPONSE 0x0008
#define DOIP_DOIP_ENTITY_STATUS_REQUEST 0x4001
#define DOIP_DOIP_ENTITY_STATUS_RESPONSE 0x4002
#define DOIP_DIAGNOSTIC_POWER_MODE_INFORMATION_REQUEST 0x4003
#define DOIP_DIAGNOSTIC_POWER_MODE_INFORMATION_RESPONSE 0x4004
#define DOIP_DIAGNOSTIC_MESSAGE 0x8001
#define DOIP_DIAGNOSTIC_ACK 0x8002
#define DOIP_DIAGNOSTIC_NACK 0x8003

#define DOIP_ROUTE_ACTIVATION_RESERVED_ISO13400_LENGTH 4
#define DOIP_ROUTE_ACTIVATION_RESERVED_OEM_LENGTH 4
#define DOIP_ROUTE_ACTIVATION_RESPONSE_MIN_LENGTH 9

#define DOIP_ROUTING_ACTIVATION_DENIED_UNKNOWN_SA 0x00
#define DOIP_ROUTING_ACTIVATION_DENIED_ALL_SOCKETS_REGISTERED 0x01
#define DOIP_ROUTING_ACTIVATION_DENIED_SA_DIFFERENT 0x02
#define DOIP_ROUTING_ACTIVATION_DENIED_SA_ALREADY_REGISTERED_AND_ACTIVE 0x03
#define DOIP_ROUTING_ACTIVATION_DENIED_MISSING_AUTHENTICATION 0x04
#define DOIP_ROUTING_ACTIVATION_DENIED_REJECTED_CONFIRMATION 0x05
#define DOIP_ROUTING_ACTIVATION_DENIED_UNSUPPORTED_ROUTING_ACTIVATION_TYPE 0x06
#define DOIP_ROUTING_ACTIVATION_SUCCESSFULLY_ACTIVATED 0x10
#define DOIP_ROUTING_ACTIVATION_WILL_ACTIVATED_CONFIRMATION_REQUIRED 0x11

const int _NACKLength = 1;

const unsigned char _IncorrectPatternFormatCode = 0x00;
const unsigned char _UnknownPayloadTypeCode = 0x01;
const unsigned char _InvalidPayloadLengthCode = 0x04;

enum PayloadType {
    NEGATIVEACK,
    ROUTINGACTIVATIONREQUEST,
    ROUTINGACTIVATIONRESPONSE,
    VEHICLEIDENTREQUEST,
    VEHICLEIDENTRESPONSE,
    DIAGNOSTICMESSAGE,
    DIAGNOSTICPOSITIVEACK,
    DIAGNOSTICNEGATIVEACK,
    ALIVECHECKRESPONSE,
};

enum class ByteOrder : uint8_t{
    kHost,
    kNetwork
};

struct DoIPHeader {
   uint8_t protocol_version_;
   uint8_t inv_protocol_version_;
   uint16_t payload_type_;
   uint32_t payload_length_;

   DoIPHeader(){}
   DoIPHeader(const QByteArray& data)
   {
       if(data.size() >= DOIP_GENERIC_HEADER_LENGTH)
       {
            int pos = 0;
            protocol_version_ = (uint8_t)data.at(pos++);
            inv_protocol_version_ = (uint8_t)data.at(pos++);

            payload_type_ = ((uint8_t)data.at(pos++) << 8);
            payload_type_ += (uint8_t)data.at(pos++);

            payload_length_ = ((uint8_t)data.at(pos++) << 24);
            payload_length_ += ((uint8_t)data.at(pos++) << 16);
            payload_length_ += ((uint8_t)data.at(pos++) << 8);
            payload_length_ += (uint8_t)data.at(pos++);
       }
   }
   DoIPHeader(uint8_t ver, uint16_t payload_type, uint32_t payload_length)
   :protocol_version_(ver)
   ,inv_protocol_version_(~ver)
   ,payload_type_(payload_type)
   ,payload_length_(payload_length)
   {
   }

   //设置版本信息
   void SetProtocolVersion(uint8_t ver)
   {
       protocol_version_ = ver;
       inv_protocol_version_ = ~ver;
   }

   QByteArray GetArray() const
   {
       QByteArray array;
       array.append(protocol_version_);
       array.append(inv_protocol_version_);
       array.append(static_cast<char>((payload_type_ >> 8) & 0xFF));
       array.append(static_cast<char>(payload_type_ & 0xFF));
       array.append(static_cast<char>((payload_length_ >> 24) & 0xFF));
       array.append(static_cast<char>((payload_length_ >> 16) & 0xFF));
       array.append(static_cast<char>((payload_length_ >> 8) & 0xFF));
       array.append(static_cast<char>(payload_length_ & 0xFF));

       return array;
   }

};

struct UdsPayloadMessage {
    uint16_t source_address_;
    uint16_t target_address_;
    QByteArray user_data_;
};

class DoIPPacketCommon
{

private:
    ByteOrder byte_order_ { ByteOrder::kHost };
    DoIPHeader doip_header_;
    QByteArray payload_message_;
    UdsPayloadMessage message_;

public:
    DoIPPacketCommon();
    DoIPPacketCommon(const QByteArray& data);
    ~DoIPPacketCommon() = default;

    //获取协议头
    const DoIPHeader& GetDoIPHeader();

    //设置协议版本
    void SetProtocolVersion(uint8_t ver);
    //获取协议版本
    uint8_t GetProtocolVersion();
    //获取协议反向版本
    uint8_t GetProtocolInverseVersion();

    //设置负载类型
    void SetPayloadType(uint16_t type);
    //获取负载类型
    uint16_t GetPayloadType();

    //获取负载长度
    uint32_t GetPayloadLength();
    //设置负载长度
    void SetPayloadLength(uint32_t length);

    //设置负载数据 源地址、目标地址
    void SetDiagnosticMessage(const QByteArray& user_data);
    void SetPayloadMessage(const QByteArray& payload_msg);

    //获取负载数据
    const QByteArray& GetPayloadMessage() const;
    //UdsPayloadMessage& GetUdsPayloadMessage();

    // ScatterArray GetScatterArray();

    //获取协议完整数据
    QByteArray GetDoIPMessage();

    //主机转网络字节
    void Hton();
    //网络转主机字节
    void Ntoh();

public:
    static QByteArray ConstructVehicleIdentificationRequest();

    /**
     * 构造路由激活指令
     * header: ver[1] + ~ver[1] + payload_type[2] + payload_lenght[4]
     * payload: SA[2] + activation_type[1] + reserve[4] + oem[4]
     * */
    static QByteArray ConstructRoutingActivationRequest();

    /**
     * 构造诊断请求指令
     * header: ver[1] + ~ver[1] + payload_type[2] + payload_lenght[4]
     * payload: SA[2] + TA[2] + user_data[payload_lenght-4]
     * \param SA source address
     * \param TA targete address
     * \param user_data user given UDS data
     * */
    static QByteArray ConstructDiagnosticMessageRequest(const QByteArray& user_data);
    /**
     * 构造AliveCheck响应指令
     * header: ver[1] + ~ver[1] + payload_type[2] + payload_lenght[4]
     * payload: SA[2]
     * \param SA source address
     * */
    static QByteArray ConstructAliveCheckResponse();
//    static DoipHeader ParseDoipHeaderOnly(const std::vector<uint8_t>& doip_message);
//    static UdsPayloadMessage ParseDoipUdsPayloadOnly(const std::vector<uint8_t>& doip_payload);
//    static RoutingActivationInfo ParseRoutingActivationResponse(const std::vector<uint8_t>& payload);
};

struct GenericHeaderAction {
    PayloadType type;
    unsigned char value;
    unsigned long payloadLength;
};

GenericHeaderAction parseGenericHeader(unsigned char* data, int dataLenght);
QByteArray createGenericHeader(PayloadType type, int length);


#endif /* DOIPGENERICHEADERHANDLER_H */

