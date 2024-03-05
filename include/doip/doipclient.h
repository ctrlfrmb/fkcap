/**
 * @file    doipclient.h
 * @ingroup opensource
 * @brief
 * @author  leiwei
 * @date    2024.03.04
 * Copyright (c) ctrlfrmb 2024-2034
 */

#pragma once

#ifndef DOIPCLIENT_H
#define DOIPCLIENT_H

#include <iostream>
#include <cstddef>
#include <stdlib.h>
#include <cstdlib>
#include <cstring>
#include "common/tcpclient.h"
#include "common/udpclient.h"
#include "diagnosticmessagehandler.h"
#include "doipgenericheaderhandler.h"

const int _serverPortNr=13400;
const int _maxDataSize=64;

namespace figkey {

class DoIPClient{
public:
    DoIPClient();

    void SetMessageCallback(const opensource::ctrlfrmb::HandleTCPMessage& callback);

    bool startTcpConnection();
    bool startUdpConnection();
    bool sendRoutingActivationRequest();
    void sendVehicleIdentificationRequest(const char* address);
    void receiveRoutingActivationResponse();
    void receiveUdpMessage();
    void receiveMessage(std::vector<uint8_t> data);
    bool sendDiagnosticMessage(const std::vector<uint8_t>& data);
    bool sendAliveCheckResponse();
    void setSourceAddress(unsigned char* address);
    void displayVIResponseInformation();
    void closeTcpConnection();
    void closeUdpConnection();
    void reconnectServer();

private:
    unsigned char _receivedData[_maxDataSize];
    //int _sockFd, _sockFd_udp, _connected;
    unsigned short sourceAddress{0x0E01};
    unsigned short targetAddress{0xBBBB};
    opensource::ctrlfrmb::HandleTCPMessage diagnosticCallback;
    opensource::ctrlfrmb::TCPClient tc;
    opensource::ctrlfrmb::UDPClient uc;

    int broadcast = 1;
    struct sockaddr_in _serverAddr, _clientAddr; 
    //unsigned char sourceAddress [2];
    
    unsigned char VINResult [17];
    unsigned char LogicalAddressResult [2];
    unsigned char EIDResult [6];
    unsigned char GIDResult [6];
    unsigned char FurtherActionReqResult;
    
    std::vector<uint8_t> buildRoutingActivationRequest();
    const std::pair<int, unsigned char*>* buildVehicleIdentificationRequest();
    void parseVIResponseInformation(unsigned char* data);
    
    int emptyMessageCounter = 0;
};

}

#endif /* DOIPCLIENT_H */

