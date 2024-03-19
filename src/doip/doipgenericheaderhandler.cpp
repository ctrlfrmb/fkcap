#include <iostream>
#include <iomanip>
#include <QDataStream>
#include <QIODevice>
#include <QBuffer>
#include <QtEndian>

#include "doip/doipgenericheaderhandler.h"
#include "doip/doipclientconfig.h"

//using namespace std;

/**
 * Checks if the received Generic Header is valid
 * @param data          message which was received
 * @param dataLenght    length of the message
 * @return              Returns a GenericHeaderAction struct, which stores the
 *                      payload type and a byte for further message processing
 */
GenericHeaderAction parseGenericHeader(unsigned char* data, int dataLenght) {
    
    GenericHeaderAction action;
    
    //Only check header if received data is greater or equals the set header length
    if(dataLenght >= DOIP_GENERIC_HEADER_LENGTH) {
        
        //Check Generic DoIP synchronization pattern
        if((int)(data[1] ^ (0xFF)) != (int)data[0]) {
            //Return Error, Protocol Version not correct
            action.type = PayloadType::NEGATIVEACK;
            action.value = _IncorrectPatternFormatCode;
            action.payloadLength = 0;
            return action;
        }

        unsigned int payloadLength = 0;
        payloadLength |= (unsigned int)(data[4] << 24);
        payloadLength |= (unsigned int)(data[5] << 16);
        payloadLength |= (unsigned int)(data[6] <<  8);
        payloadLength |= (unsigned int)(data[7] <<  0);

        action.payloadLength = payloadLength;

        //Check Payload Type
        PayloadType messagePayloadType;
        if(data[2] == 0x00 && data[3] == 0x05) {	//Value of RoutingActivationRequest = 0x0005
            messagePayloadType = PayloadType::ROUTINGACTIVATIONREQUEST;
        }
        else if(data[2] == 0x00 && data[3] == 0x04){
            messagePayloadType = PayloadType::VEHICLEIDENTRESPONSE;
        }
        else if(data[2] == 0x00 && data[3] == 0x01) {   //Value of Vehicle Identification Request = 0x0001
            messagePayloadType = PayloadType::VEHICLEIDENTREQUEST;
        }
        else if(data[2] == 0x80 && data[3] == 0x01) {   //Value of Diagnose Message = 0x8001
            messagePayloadType = PayloadType::DIAGNOSTICMESSAGE;
        } 
        else if(data[2] == 0x80 && data[3] == 0x02) {   //Value of Diagnostic Message positive ack = 0x8002
            messagePayloadType = PayloadType::DIAGNOSTICPOSITIVEACK;
        } 
        else if(data[2] == 0x80 && data[3] == 0x03) {   //Value of Diagnostic Message negative ack = 0x8003
            messagePayloadType = PayloadType::DIAGNOSTICNEGATIVEACK;
        } else {
            //Unknown Payload Type --> Send Generic DoIP Header NACK
            action.type = PayloadType::NEGATIVEACK;
            action.value = _UnknownPayloadTypeCode;
            return action;
        }

        //Check Payload Type specific length
        switch(messagePayloadType) {
            case PayloadType::ROUTINGACTIVATIONREQUEST: {
                if(payloadLength != 7 && payloadLength != 11) {
                    action.type = PayloadType::NEGATIVEACK;
                    action.value = _InvalidPayloadLengthCode;
                    return action;
                }
                break;
            }

            case PayloadType::ALIVECHECKRESPONSE: {
                if(payloadLength != 2) {
                    action.type = PayloadType::NEGATIVEACK;
                    action.value = _InvalidPayloadLengthCode;
                    return action;
                }
                break;
            }
            
            case PayloadType::VEHICLEIDENTREQUEST: {
                if(payloadLength != 0) {
                    action.type = PayloadType::NEGATIVEACK;
                    action.value = _InvalidPayloadLengthCode;
                    return action;
                }
                break;
            }

            case PayloadType::VEHICLEIDENTRESPONSE:{
                if(payloadLength != 32 && payloadLength != 33) {
                    action.type = PayloadType::NEGATIVEACK;
                    action.value = _InvalidPayloadLengthCode;
                    return action;
                }
                break;
            }

            case PayloadType::DIAGNOSTICMESSAGE: {
                if(payloadLength <= 4) {
                    action.type = PayloadType::NEGATIVEACK;
                    action.value = _InvalidPayloadLengthCode;
                    return action;
                }
                break;	
            }

            case PayloadType::DIAGNOSTICPOSITIVEACK: {
                if(payloadLength < 5) {
                    action.type = PayloadType::NEGATIVEACK;
                    action.value = _InvalidPayloadLengthCode;
                }
                break;
            }

            case PayloadType::DIAGNOSTICNEGATIVEACK: {
                if(payloadLength < 5) {
                    action.type = PayloadType::NEGATIVEACK;
                    action.value = _InvalidPayloadLengthCode;
                }
                break;
            }

            default: {
                std::cerr << "not handled payload type occured in parseGenericHeader()" << std::endl;
                break;	
            }
        }
        action.type = messagePayloadType;
    }
    
    return action;
}

/**
 * Creates a generic header
 * @param type      payload type which will be filled in the header
 * @param length    length of the payload type specific message
 * @return          header array
 */
QByteArray createGenericHeader(PayloadType type, int length) {
    QByteArray header; header.resize(8);

    // Get configuration
    auto& config = figkey::DoIPClientConfig::Instance();
    auto ver = config.getVersion();

    //Generic Header
    header[0]=  (ver & 0x000000FF);  //Protocol Version
    header[1]= ~(ver & 0x000000FF);  //Inverse Protocol Version

    switch(type) {
        case PayloadType::ROUTINGACTIVATIONRESPONSE: {
            header[2] = 0x00;
            header[3] = 0x06;
            break;
        }

        case PayloadType::NEGATIVEACK: {
            header[2] = 0x00;
            header[3] = 0x00;
            break;
        }

        case PayloadType::VEHICLEIDENTRESPONSE:{
            header[2] = 0x00;
            header[3] = 0x04;
            break;
        }

        case PayloadType::DIAGNOSTICMESSAGE: {
            header[2] = 0x80;
            header[3] = 0x01;
            break;
        }

        case PayloadType::DIAGNOSTICPOSITIVEACK: {
            header[2] = 0x80;
            header[3] = 0x02;
            break;
        }

        case PayloadType::DIAGNOSTICNEGATIVEACK: {
            header[2] = 0x80;
            header[3] = 0x03;
            break;
        }

        case PayloadType::ALIVECHECKRESPONSE: {
            header[2] = 0x00;
            header[3] = 0x08;
            break;
        }

        default: {
            std::cerr << "not handled payload type occured in createGenericHeader()" << std::endl;
            break;
        }
    }

    header[4] = (length >> 24) & 0xFF;
    header[5] = (length >> 16) & 0xFF;
    header[6] = (length >> 8) & 0xFF;
    header[7] = length & 0xFF;

    return header;
}

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
    // Get configuration
    auto& config = figkey::DoIPClientConfig::Instance();
    auto ver = config.getVersion();
    packet.SetProtocolVersion(ver);
    packet.SetPayloadType(DOIP_VEHICLE_IDENTIFICATION_REQUEST);

    return packet.GetDoIPMessage();

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
