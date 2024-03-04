#ifndef DOIPGENERICHEADERHANDLER_H
#define DOIPGENERICHEADERHANDLER_H

#include <stdint.h>
#include <vector>

const int _GenericHeaderLength = 8;
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

struct GenericHeaderAction {
    PayloadType type;
    unsigned char value;
    unsigned long payloadLength;
};

GenericHeaderAction parseGenericHeader(unsigned char* data, int dataLenght);
std::vector<uint8_t> createGenericHeader(PayloadType type, size_t length);


#endif /* DOIPGENERICHEADERHANDLER_H */

