#include <iostream>
#include <iomanip>
#include <QDataStream>
#include <QIODevice>
#include <QBuffer>
#include <QtEndian>

#include "doip/doipgenericheaderhandler.h"
#include "doip/doipclientconfig.h"
#include "doip/doipserverconfig.h"

DoIPPacketCommon::DoIPPacketCommon()
    : doip_header_(DOIP_PROTOCOL_VERSION_MAX, 0, 0)
{
}

DoIPPacketCommon::DoIPPacketCommon(const QByteArray& data)
    : byte_order_(ByteOrder::kHost)
{
    if (data.size() >= DOIP_GENERIC_HEADER_LENGTH)
    {
        doip_header_ = DoIPHeader(data);
    }

    if (doip_header_.payload_length_ > 0)
    {
        payload_message_ = data.mid(DOIP_GENERIC_HEADER_LENGTH);
    }
}

//获取协议头
const DoIPHeader& DoIPPacketCommon::GetDoIPHeader()
{
    return doip_header_;
}

//设置协议版本
void DoIPPacketCommon::SetProtocolVersion(uint8_t ver)
{
    doip_header_.SetProtocolVersion(ver);
}

//获取协议版本
uint8_t DoIPPacketCommon::GetProtocolVersion()
{
    return doip_header_.protocol_version_;
}

//获取协议反向版本
uint8_t DoIPPacketCommon::GetProtocolInverseVersion()
{
    return doip_header_.inv_protocol_version_;
}

//设置负载类型
void DoIPPacketCommon::SetPayloadType(uint16_t type)
{
    doip_header_.payload_type_ = type;
}

//获取负载类型
uint16_t DoIPPacketCommon::GetPayloadType()
{
    return doip_header_.payload_type_;
}

//获取负载长度
uint32_t DoIPPacketCommon::GetPayloadLength()
{
    return payload_message_.size();
}

//设置负载长度
void DoIPPacketCommon::SetPayloadLength(uint32_t length)
{
    payload_message_.resize(length);
    doip_header_.payload_length_ = payload_message_.size();
}

//设置负载数据 源地址、目标地址
void DoIPPacketCommon::SetDiagnosticMessage(const QByteArray& user_data)
{
    if (user_data.isEmpty())
    {
        return;
    }

    auto& config = figkey::DoIPClientConfig::Instance();
    auto sourceAddress = config.getSourceAddress();
    // Add source address to the message
    QByteArray payload;
    payload.append((sourceAddress >> 8) & 0xFF);
    payload.append(sourceAddress & 0xFF);

    // Add target address to the message
    auto targetAddress = config.getTargetAddress();
    payload.append((targetAddress >> 8) & 0xFF);
    payload.append(targetAddress & 0xFF);

    // Add userdata to the message
    payload.append(user_data);
    payload_message_ = payload;
    doip_header_.payload_length_ = payload_message_.size();
}

////设置负载数据
//void DoIPPacketCommon::SetPayloadMessage(const std::vector<uint8_t>& payload_msg)
//{
//    if (payload_msg.empty())
//    {
//        return;
//    }
//    // payload_length_ = payload_msg.size();
//    // payload_message_.resize(payload_length_, 0);
//    // std::copy(payload_msg.begin(), payload_msg.end(), payload_message_.begin());
//    payload_message_.resize(payload_msg.size(), 0);
//    std::copy(payload_msg.begin(), payload_msg.end(), payload_message_.begin());
//    doip_header_.payload_length_ = payload_message_.size();
//}

//设置负载数据
void DoIPPacketCommon::SetPayloadMessage(const QByteArray& payload_msg)
{
    if (payload_msg.isEmpty())
    {
        return;
    }

    payload_message_ = payload_msg;
    doip_header_.payload_length_ = payload_message_.size();
}

//获取负载数据
 const QByteArray& DoIPPacketCommon::GetPayloadMessage() const
{
    return payload_message_;
}

////获取负载数据
//UdsPayloadMessage& DoIPPacketCommon::GetUdsPayloadMessage()
//{
//    if (payload_message_.size() >= kDoIp_DiagnosticMessage_length_min)
//    {
//        int pos = 0;
//        message_.source_address_ = payload_message_.at(pos) * 0x100 + payload_message_.at(pos + 1);
//        pos += 2;
//        message_.target_address_ = payload_message_.at(pos) * 0x100 + payload_message_.at(pos + 1);
//        pos += 2;
//        message_.user_data_.resize(payload_message_.size() - pos, 0);
//        std::copy(payload_message_.begin() + pos, payload_message_.end(), message_.user_data_.begin());
//    }
//    return message_;
//}

// DoipPacketCommon::ScatterArray DoipPacketCommon::GetScatterArray()
// {
//     ScatterArray scatter_array;

//     scatter_array[kProtocolVersionIdx].iov_base = &doip_header_.protocol_version_;
//     scatter_array[kProtocolVersionIdx].iov_len = kDoIp_ProtocolVersion_length;
//     scatter_array[kInvProtocolVersionIdx].iov_base = &doip_header_.inv_protocol_version_;
//     scatter_array[kInvProtocolVersionIdx].iov_len = kDoIp_InvProtocolVersion_length;
//     scatter_array[kPayloadTypeIdx].iov_base = &doip_header_.payload_type_;
//     scatter_array[kPayloadTypeIdx].iov_len = kDoIp_PayloadType_length;
//     scatter_array[kPayloadLengthIdx].iov_base = &doip_header_.payload_length_;
//     scatter_array[kPayloadLengthIdx].iov_len = kDoIp_PayloadLength_length;
//     scatter_array[kPayloadIdx].iov_base = payload_message_.data();
//     scatter_array[kPayloadIdx].iov_len = payload_message_.size();

//     return scatter_array;
// }

//获取协议完整数据
QByteArray DoIPPacketCommon::GetDoIPMessage()
{
    if (payload_message_.isEmpty())
        return doip_header_.GetArray();

    QByteArray messsage(doip_header_.GetArray());
    return messsage.append(payload_message_);
}

void DoIPPacketCommon::Hton()
{
    if (byte_order_ != ByteOrder::kNetwork)
    {
        QByteArray buffer(4, '\0');

        // For payload type
        QBuffer bufferDevice(&buffer);
        bufferDevice.open(QIODevice::ReadWrite);
        QDataStream stream(&bufferDevice);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << doip_header_.payload_type_;
        doip_header_.payload_type_ = qFromBigEndian<quint16>((const uchar*)buffer.constData());

        // For payload length
        bufferDevice.seek(0);
        stream << doip_header_.payload_length_;
        doip_header_.payload_length_ = qFromBigEndian<quint32>((const uchar*)buffer.constData());

        byte_order_ = ByteOrder::kNetwork;
    }
}

void DoIPPacketCommon::Ntoh()
{
    if (byte_order_ != ByteOrder::kHost)
    {
        QByteArray buffer(4, '\0');

        // For payload type
        QBuffer bufferDevice(&buffer);
        bufferDevice.open(QIODevice::ReadWrite);
        QDataStream stream(&bufferDevice);
        stream.setByteOrder(QDataStream::BigEndian);
        stream << doip_header_.payload_type_;
        doip_header_.payload_type_ = qFromLittleEndian<quint16>((const uchar*)buffer.constData());

        // For payload length
        bufferDevice.seek(0);
        stream << doip_header_.payload_length_;
        doip_header_.payload_length_ = qFromLittleEndian<quint32>((const uchar*)buffer.constData());

        byte_order_ = ByteOrder::kHost;
    }
}

QByteArray DoIPPacketCommon::ConstructVehicleIdentificationRequest() {
    DoIPPacketCommon packet;

    packet.SetProtocolVersion(figkey::DoIPClientConfig::Instance().getVersion());
    packet.SetPayloadType(DOIP_VEHICLE_IDENTIFICATION_REQUEST);

    return packet.GetDoIPMessage();
}

QByteArray DoIPPacketCommon::ConstructVehicleIdentificationRequestWithEid() {
    DoIPPacketCommon packet;

    packet.SetProtocolVersion(figkey::DoIPClientConfig::Instance().getVersion());
    packet.SetPayloadType(DOIP_VEHICLE_IDENTIFICATION_REQUEST_WITH_EID);
    packet.SetPayloadMessage(figkey::DoIPServerConfig::Instance().getEid());

    return packet.GetDoIPMessage();
}

QByteArray DoIPPacketCommon::ConstructVehicleIdentificationRequestWithEid(const QByteArray &eid) {
    DoIPPacketCommon packet;
    if (eid.size() != DOIP_VEHICLE_IDENTIFICATION_REQUEST_WITH_EID_LENGTH) {
        return {};
    }

    packet.SetProtocolVersion(figkey::DoIPClientConfig::Instance().getVersion());
    packet.SetPayloadType(DOIP_VEHICLE_IDENTIFICATION_REQUEST_WITH_EID);
    packet.SetPayloadMessage(eid);

    return packet.GetDoIPMessage();
}

QByteArray DoIPPacketCommon::ConstructVehicleIdentificationRequestWithVin() {
    DoIPPacketCommon packet;

    packet.SetProtocolVersion(figkey::DoIPClientConfig::Instance().getVersion());
    packet.SetPayloadType(DOIP_VEHICLE_IDENTIFICATION_REQUEST_WITH_VIN);
    packet.SetPayloadMessage(figkey::DoIPServerConfig::Instance().getVin());

    return packet.GetDoIPMessage();
}

QByteArray DoIPPacketCommon::ConstructVehicleIdentificationRequestWithVin(const QByteArray &vin) {
    DoIPPacketCommon packet;
    if (vin.size() != DOIP_VEHICLE_IDENTIFICATION_REQUEST_WITH_VIN_LENGTH) {
        return {};
    }

    packet.SetProtocolVersion(figkey::DoIPClientConfig::Instance().getVersion());
    packet.SetPayloadType(DOIP_VEHICLE_IDENTIFICATION_REQUEST_WITH_VIN);
    packet.SetPayloadMessage(vin);

    return packet.GetDoIPMessage();
}

QMap<QString, QString> DoIPPacketCommon::ParseVehicleAnnouncementInformation(const QByteArray &payload)
{
    QMap<QString, QString> info;

    int minPayloadSize = 32;
    if (payload.size() < minPayloadSize) {
        return info;
    }

    int currentIndex = 0;

    QByteArray vinByteArray = payload.mid(currentIndex, 17);
    currentIndex += 17;
    info.insert(DOIP_VEHICLE_VIN_ATTRIBUTE, QString::fromUtf8(vinByteArray.toHex(' ')));

    QByteArray logicalAddressBytes = payload.mid(currentIndex, 2);
    uint logicalAddress = ((uchar)logicalAddressBytes[0] << 8) | (uchar)logicalAddressBytes[1];
    currentIndex += 2;
    info.insert(DOIP_VEHICLE_LOGIC_ADDRESS_ATTRIBUTE, QString::number(logicalAddress, 16));

    QByteArray eidByteArray = payload.mid(currentIndex, 6);
    currentIndex += 6;
    info.insert(DOIP_VEHICLE_EID_ATTRIBUTE, QString::fromUtf8(eidByteArray.toHex(' ')));

    QByteArray gidByteArray = payload.mid(currentIndex, 6);
    currentIndex += 6;
    info.insert(DOIP_VEHICLE_GID_ATTRIBUTE, QString::fromUtf8(gidByteArray.toHex(' ')));

    if (payload.size() == DOIP_VEHICLE_ANNOUNCEMENT_MAX_LENGTH) {
        info.insert(DOIP_VEHICLE_SYNC_STATUS_ATTRIBUTE, QString::number((uchar)payload.at(currentIndex)));
    }

    return info;
}

QByteArray DoIPPacketCommon::ConstructDoipEntityStatusRequest() {
    DoIPPacketCommon packet;

    packet.SetProtocolVersion(figkey::DoIPClientConfig::Instance().getVersion());
    packet.SetPayloadType(DOIP_DOIP_ENTITY_STATUS_REQUEST);

    return packet.GetDoIPMessage();
}

QMap<QString, QString> DoIPPacketCommon::ParseDoIPEntityStatus(const QByteArray &payload)
{
    QMap<QString, QString> status;

    if (payload.size() < DOIP_ENTITY_STATUS_RESPONSE_MIN_LENGTH) {
        return status;
    }

    switch (payload[0]) {
    case 0x00:
        status.insert(DOIP_ENTITY_NODE_TYPE_ATTRIBUTE, "DoIP gateway");
        break;
    case 0x01:
        status.insert(DOIP_ENTITY_NODE_TYPE_ATTRIBUTE, "DoIP node");
        break;
    default:
        status.insert(DOIP_ENTITY_NODE_TYPE_ATTRIBUTE, QString::number((quint8)payload[0], 16));
        break;
    }

    status.insert(DOIP_ENTITY_MCTS_ATTRIBUTE, QString::number((quint8)payload[1]));
    status.insert(DOIP_ENTITY_NCTS_ATTRIBUTE, QString::number((quint8)payload[2]));

    if (payload.size() == DOIP_ENTITY_STATUS_RESPONSE_MAX_LENGTH) {
        quint32 MDS = ((quint8)payload[3] << 24) | ((quint8)payload[4] << 16) | ((quint8)payload[5] << 8) | (quint8)payload[6];
        status.insert(DOIP_ENTITY_MDS_ATTRIBUTE, QString::number(MDS));
    }
    return status;
}

QByteArray DoIPPacketCommon::ConstructDiagnosticPowerModeInformationRequest() {
    DoIPPacketCommon packet;

    packet.SetProtocolVersion(figkey::DoIPClientConfig::Instance().getVersion());
    packet.SetPayloadType(DOIP_DIAGNOSTIC_POWER_MODE_INFORMATION_REQUEST);

    return packet.GetDoIPMessage();
}

QMap<QString, QString> DoIPPacketCommon::ParseDiagnosticPowerModeInformation(const QByteArray &payload) {
    QMap<QString, QString> mode;

    if (payload.size() != DOIP_DIAGNOSTIC_POWER_MODE_INFORMATION_RESPONSE_LENGTH) {
        return mode;
    }

    switch (payload[0]) {
    case 0x00:
        mode.insert(DOIP_DIAGNOSTIC_POWER_MODE_ATTRIBUTE, "not ready");
        break;
    case 0x01:
        mode.insert(DOIP_DIAGNOSTIC_POWER_MODE_ATTRIBUTE, "ready");
        break;
    case 0x02:
        mode.insert(DOIP_DIAGNOSTIC_POWER_MODE_ATTRIBUTE, "not supported");
        break;
    default:
        mode.insert(DOIP_DIAGNOSTIC_POWER_MODE_ATTRIBUTE, QString::number((quint8)payload[0], 16));
        break;
    }
    return mode;
}

/**
    * 构造路由激活指令
    * header: ver[1] + ~ver[1] + payload_type[2] + payload_lenght[4]
    * payload: SA[2] + activation_type[1] + reserve[4] + oem[4]
    * */
QByteArray DoIPPacketCommon::ConstructRoutingActivationRequest()
{
    DoIPPacketCommon packet;
    // Get configuration
    auto& config = figkey::DoIPClientConfig::Instance();
    auto ver = config.getVersion();
    packet.SetProtocolVersion(ver);
    packet.SetPayloadType(DOIP_ROUTING_ACTIVATION_REQUEST);

    auto sourceAddress = config.getSourceAddress();
    QByteArray payload;
    payload.append((sourceAddress >> 8) & 0xFF);
    payload.append(sourceAddress & 0xFF);
    payload.append(config.getActiveType());

    auto future = config.getFutureStandardization();
    if (future.size() == DOIP_ROUTE_ACTIVATION_RESERVED_ISO13400_LENGTH) {
        payload.append(future);
    }
    else {
        payload.append(DOIP_ROUTE_ACTIVATION_RESERVED_ISO13400_LENGTH, 0);
    }
    if (config.getUseOEMSpecific()) {
        auto opt = config.getAdditionalOEMSpecific();
        if (opt.size() == DOIP_ROUTE_ACTIVATION_RESERVED_OEM_LENGTH) {
            payload.append(opt);
        } else {
            payload.append(DOIP_ROUTE_ACTIVATION_RESERVED_OEM_LENGTH, 0);
        }
    }

    packet.SetPayloadMessage(payload);
    return packet.GetDoIPMessage();
}

/**
    * 构造诊断请求指令
    * header: ver[1] + ~ver[1] + payload_type[2] + payload_lenght[4]
    * payload: SA[2] + TA[2] + user_data[payload_lenght-4]
    * \param SA source address
    * \param TA targete address
    * \param user_data user given UDS data
    * */
QByteArray DoIPPacketCommon::ConstructDiagnosticMessageRequest(const QByteArray& user_data)
{
    DoIPPacketCommon packet;
    // Get configuration
    auto& config = figkey::DoIPClientConfig::Instance();
    auto ver = config.getVersion();
    packet.SetProtocolVersion(ver);
    packet.SetPayloadType(DOIP_DIAGNOSTIC_MESSAGE);
    packet.SetDiagnosticMessage(user_data);

    return packet.GetDoIPMessage();
}

/**
    * 构造AliveCheck响应指令
    * header: ver[1] + ~ver[1] + payload_type[2] + payload_lenght[4]
    * payload: SA[2]
    * \param SA source address
    * */
QByteArray DoIPPacketCommon::ConstructAliveCheckResponse()
{
    DoIPPacketCommon packet;
    // Get configuration
    auto& config = figkey::DoIPClientConfig::Instance();
    auto ver = config.getVersion();
    packet.SetProtocolVersion(ver);
    packet.SetPayloadType(DOIP_ALIVE_CHECK_RESPONSE);

    auto sourceAddress = config.getSourceAddress();
    QByteArray payload;
    payload.append((sourceAddress >> 8) & 0xFF);
    payload.append(sourceAddress & 0xFF);

    packet.SetPayloadMessage(payload);
    return packet.GetDoIPMessage();
}


//DoipHeader DoipPacketCommon::ParseDoipHeaderOnly(const std::vector<uint8_t>& doip_message)
//{
//	DoipHeader doip_header;
//	DoipPacketCommon packet(ByteOrder::kNetwork);
//	uint32_t message_len = doip_message.size();
//	if (message_len == kDoIp_HeaderTotal_length)
//	{
//		std::copy(doip_message.begin(), doip_message.end(), (uint8_t*)&packet.doip_header_);
//	}
//	else if (message_len > kDoIp_HeaderTotal_length)
//	{
//		std::copy(doip_message.begin(), doip_message.begin() + kDoIp_HeaderTotal_length, (uint8_t*)&packet.doip_header_);
//	}
//	else
//	{
//		return doip_header;
//	}
//	packet.Ntoh();
//	doip_header = std::move(packet.doip_header_);
//	return doip_header;
//}

////srcAddr[2] + tgtAddr[2] + userData[n-4]
//UdsPayloadMessage DoipPacketCommon::ParseDoipUdsPayloadOnly(const std::vector<uint8_t>& doip_payload)
//{
//	UdsPayloadMessage result;
//	int pos = 0;
//	result.source_address_ = doip_payload.at(pos++) * 0x100;
//	result.source_address_ += doip_payload.at(pos++);
//	result.target_address_ = doip_payload.at(pos++) * 0x100;
//	result.target_address_ += doip_payload.at(pos++);
//	result.user_data_.resize(doip_payload.size() - pos, 0);
//	auto first = doip_payload.begin() + pos;
//	std::copy(first, doip_payload.end(), result.user_data_.begin());
//	return result;
//}

////testerAddr[2] + doipAddr[2] + code[1] + standardization[4] [ + OEM_specific[4] ]
//RoutingActivationInfo DoipPacketCommon::ParseRoutingActivationResponse(const std::vector<uint8_t>& payload)
//{
//	// std::cout << "" << std::endl;
//	// logp_.LogDebug() << "RA payload " << payload;
//	RoutingActivationInfo info{};
//	if (payload.size() < kDoIp_RoutingActivationResponse_length_min
//		|| payload.size() > kDoIp_RoutingActivationResponse_length_max)
//	{
//		return info;
//	}
//	int pos = 0;
//	info.LogicAddrTester = payload.at(pos) * 0x100;
//	++pos;
//	info.LogicAddrTester += payload.at(pos);
//	++pos;
//	info.LogicAddrDoipEntity = payload.at(pos) * 0x100;
//	++pos;
//	info.LogicAddrDoipEntity += payload.at(pos);
//	++pos;
//	info.ResponseCode = payload.at(pos);
//	++pos;
//	// logp_.LogDebug() << "TestAddr:" << HexFormat(info.LogicAddrTester)
//	// << "DoipAddr:" << HexFormat(info.LogicAddrDoipEntity)
//	// << "RespCode:" << HexFormat(info.ResponseCode);

//	auto first = payload.begin() + pos;
//	int len = info.Standardization.max_size();
//	pos += len;
//	auto last = payload.begin() + pos;
//	std::copy(first, last, info.Standardization.begin());

//	if (kDoIp_RoutingActivationResponse_length_max == payload.size())
//	{
//		info.UseOem = true;
//		first = payload.begin() + pos;
//		std::copy(first, payload.end(), info.OemSpecific.begin());
//	}
//	return info;
//}
