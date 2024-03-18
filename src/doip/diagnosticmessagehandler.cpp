#include <iostream>
#include <cstring>

#include "doip/diagnosticmessagehandler.h"
#include "doip/doipclientconfig.h"
/*
 *Build the Routing-Activation-Request for server
 */
QByteArray buildRoutingActivationRequest() {
    QByteArray array;
    array.resize(15);  // Initialize the QByteArray with total size

    // Get configuration
    auto& config = figkey::DoIPClientConfig::Instance();
    auto ver = config.getVersion();

    //Generic Header
    array[0]=  (ver & 0x000000FF);  //Protocol Version
    array[1]= ~(ver & 0x000000FF);  //Inverse Protocol Version
    array[2]=0x00;  //Payload-Type
    array[3]=0x05;

    //Payload-Type specific message-content
    auto sourceAddress = config.getSourceAddress();
    array[8]  = (sourceAddress >> 8) & 0xFF; //Source Address
    array[9]  = sourceAddress & 0xFF;
    array[10] = config.getActiveType();
    // Reserved ISO (default)
    auto future = config.getFutureStandardization();
    if (future.size() == DOIP_ROUTE_ACTIVATION_RESERVED_ISO13400_LENGTH) {
        array.replace(11, 4, future);
    }
    else {
        array[14] = array[13] = array[12] = array[11] = 0x00;
    }
    if (config.getUseOEMSpecific()) {
        auto opt = config.getAdditionalOEMSpecific();
        if (opt.size() != DOIP_ROUTE_ACTIVATION_RESERVED_OEM_LENGTH) {
            std::cerr << "doip client config additional oem specific invalid, size" << opt.size() << std::endl;
        } else {
            array.append(opt);
        }
    }

    // Set Payload-Length
    auto payloadLength = array.size() - _GenericHeaderLength; // Subtracting 8 bytes of header
    array[4] = (payloadLength >> 24) & 0xFF;   // Payload-Length
    array[5] = (payloadLength >> 16) & 0xFF;
    array[6] = (payloadLength >> 8) & 0xFF;
    array[7] = payloadLength & 0xFF;

    return array;
}

/*
 *Build the Routing-Activation-Response for server
 */
QByteArray buildRoutingActivationResponse() {
    QByteArray array;
    array.resize(17);  // Initialize the QByteArray with total size

    // Get configuration
    auto& config = figkey::DoIPClientConfig::Instance();
    auto ver = config.getVersion();

    //Generic Header
    array[0]=  (ver & 0x000000FF);  //Protocol Version
    array[1]= ~(ver & 0x000000FF);  //Inverse Protocol Version
    array[2]=0x00;  //Payload-Type
    array[3]=0x06;

    // Add source address to the message
    auto sourceAddress = config.getSourceAddress();
    array[8] = (unsigned char)((sourceAddress >> 8) & 0xFF);
    array[9] = (unsigned char)(sourceAddress & 0xFF);

    // Add target address to the message
    auto targetAddress = config.getTargetAddress();
    array[10] = (unsigned char)((targetAddress >> 8) & 0xFF);
    array[11] = (unsigned char)(targetAddress & 0xFF);

    // Routing activation response code
    array[12] = DOIP_ROUTING_SUCCESSFULLY_ACTIVATED;
    auto future = config.getFutureStandardization();
    if (future.size() == DOIP_ROUTE_ACTIVATION_RESERVED_ISO13400_LENGTH) {
        array.replace(13, 4, future);
    }
    else {
        array[16] = array[15] = array[14] = array[13] = 0x00;
    }
    if (config.getUseOEMSpecific()) {
        auto opt = config.getAdditionalOEMSpecific();
        if (opt.size() != DOIP_ROUTE_ACTIVATION_RESERVED_OEM_LENGTH) {
            std::cerr << "doip client config additional oem specific invalid, size" << opt.size() << std::endl;
        } else {
            array.append(opt);
        }
    }

    // Set Payload-Length
    auto payloadLength = array.size() - _GenericHeaderLength; // Subtracting 8 bytes of header
    array[4] = (payloadLength >> 24) & 0xFF;   // Payload-Length
    array[5] = (payloadLength >> 16) & 0xFF;
    array[6] = (payloadLength >> 8) & 0xFF;
    array[7] = payloadLength & 0xFF;

    return array;
}

/**
 * Checks if a received Diagnostic Message is valid
 * @param cb                    callback which will be called with the user data
 * @param sourceAddress		currently registered source address on the socket
 * @param data			message which was received
 * @param diagMessageLength     length of the diagnostic message
 */
unsigned char parseDiagnosticMessage(DiagnosticCallback callback, unsigned char sourceAddress [2],
                                    unsigned char* data, int diagMessageLength) {
    std::cout << "parse Diagnostic Message" << std::endl;
    if(diagMessageLength >= _DiagnosticMessageMinimumLength) {
        //Check if the received SA is registered on the socket
        if(data[0] != sourceAddress[0] || data[1] != sourceAddress[1]) {
            //SA of received message is not registered on this TCP_DATA socket
            return _InvalidSourceAddressCode;
        }

        std::cout << "source address valid" << std::endl;
        //Pass the diagnostic message to the target network/transport layer
        unsigned short target_address = 0;
        target_address |= ((unsigned short)data[2]) << 8U;
        target_address |= (unsigned short)data[3];

        int cb_message_length = diagMessageLength - _DiagnosticMessageMinimumLength;
        unsigned char* cb_message = new unsigned char[cb_message_length];

        for(int i = _DiagnosticMessageMinimumLength; i < diagMessageLength; i++) {
            cb_message[i - _DiagnosticMessageMinimumLength] = data[i];
        }

        callback(target_address, cb_message, cb_message_length);

        //return positive ack code
        return _ValidDiagnosticMessageCode;
    }
    return _UnknownTargetAddressCode;
}

/**
 * Creates a diagnostic message positive/negative acknowledgment message
 * @param type                  defines positive/negative acknowledge type
 * @param sourceAddress		logical address of the receiver of the previous diagnostic message
 * @param targetAddress		logical address of the sender of the previous diagnostic message
 * @param responseCode		positive or negative acknowledge code
 * @return pointer to the created diagnostic message acknowledge
 */
QByteArray createDiagnosticACK(bool ackType, unsigned char responseCode) {

    PayloadType type = ackType ? PayloadType::DIAGNOSTICPOSITIVEACK : PayloadType::DIAGNOSTICNEGATIVEACK;

    auto message = createGenericHeader(type, _DiagnosticPositiveACKLength);

    // Add source address to the message
    auto& config = figkey::DoIPClientConfig::Instance();
    auto sourceAddress = config.getSourceAddress();
    message[8] = (unsigned char)((sourceAddress >> 8) & 0xFF);
    message[9] = (unsigned char)(sourceAddress & 0xFF);

    // Add target address to the message
    auto targetAddress = config.getTargetAddress();
    message[10] = (unsigned char)((targetAddress >> 8) & 0xFF);
    message[11] = (unsigned char)(targetAddress & 0xFF);

    // Add positive or negative acknowledge code to the message
    message[12] = responseCode;

    return message;
}

/**
 * Creates a complete diagnostic message
 * @param sourceAddress		logical address of the sender of a diagnostic message
 * @param targetAddress		logical address of the receiver of a diagnostic message
 * @param userData		actual diagnostic data
 * @param userDataLength	length of diagnostic data
 */
QByteArray createDiagnosticMessage(const QByteArray& uds) {
    auto message = createGenericHeader(PayloadType::DIAGNOSTICMESSAGE, _DiagnosticMessageMinimumLength + uds.size());

    auto& config = figkey::DoIPClientConfig::Instance();
    auto sourceAddress = config.getSourceAddress();
    // Add source address to the message
    message[8] = (unsigned char)((sourceAddress >> 8) & 0xFF);
    message[9] = (unsigned char)(sourceAddress & 0xFF);

    // Add target address to the message
    auto targetAddress = config.getTargetAddress();
    message[10] = (unsigned char)((targetAddress >> 8) & 0xFF);
    message[11] = (unsigned char)(targetAddress & 0xFF);

    // Add userdata to the message
    message.append(uds);

    return message;
}

