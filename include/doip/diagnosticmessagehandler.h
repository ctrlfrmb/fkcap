#ifndef DIAGNOSTICMESSAGEHANDLER_H
#define DIAGNOSTICMESSAGEHANDLER_H

#include "doipgenericheaderhandler.h"
#include <functional>
#include <vector>

using DiagnosticCallback = std::function<void(unsigned short, unsigned char*, int)>;
using DiagnosticMessageNotification = std::function<bool(unsigned short)>;

const int _DiagnosticPositiveACKLength = 5;
const int _DiagnosticMessageMinimumLength = 4;

const unsigned char _ValidDiagnosticMessageCode = 0x00;
const unsigned char _InvalidSourceAddressCode = 0x02;
const unsigned char _UnknownTargetAddressCode = 0x03;

unsigned char parseDiagnosticMessage(DiagnosticCallback callback, unsigned char sourceAddress [2], unsigned char* data, int diagMessageLength);
std::vector<uint8_t> createDiagnosticACK(bool ackType, unsigned short sourceAddress, short targetAddress, unsigned char responseCode);
std::vector<uint8_t> createDiagnosticMessage(unsigned short sourceAddress, unsigned short targetAddress, const std::vector<uint8_t>& uds);

#endif /* DIAGNOSTICMESSAGEHANDLER_H */
