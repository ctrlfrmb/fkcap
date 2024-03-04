#include "doip/diagnosticmessagehandler.h"
#include <iostream>
#include <cstring>

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
std::vector<uint8_t> createDiagnosticACK(bool ackType, unsigned short sourceAddress,
                                   short targetAddress, unsigned char responseCode) {
    
    PayloadType type;
    if(ackType)
        type = PayloadType::DIAGNOSTICPOSITIVEACK;
    else
        type = PayloadType::DIAGNOSTICNEGATIVEACK;
    
    auto message = createGenericHeader(type, _DiagnosticPositiveACKLength);

    //add source address to the message
    message[8] = (unsigned char)((sourceAddress >> 8) & 0xFF);
    message[9] = (unsigned char)(sourceAddress & 0xFF);

    //add target address to the message
    message[10] = (unsigned char)((targetAddress >> 8) & 0xFF);
    message[11] = (unsigned char)(targetAddress & 0xFF);

    //add positive or negative acknowledge code to the message
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
std::vector<uint8_t> createDiagnosticMessage(unsigned short sourceAddress, unsigned short targetAddress, const std::vector<uint8_t>& uds) {
    auto message = createGenericHeader(PayloadType::DIAGNOSTICMESSAGE, _DiagnosticMessageMinimumLength + uds.size());

    //add source address to the message
    message[8] = (unsigned char)((sourceAddress >> 8) & 0xFF);
    message[9] = (unsigned char)(sourceAddress & 0xFF);

    //add target address to the message
    message[10] = (unsigned char)((targetAddress >> 8) & 0xFF);
    message[11] = (unsigned char)(targetAddress & 0xFF);

    //add userdata to the message
    for(size_t i = 0; i < uds.size(); i++) {
        message[12 + i] = uds[i];
    }

    return message;
}
