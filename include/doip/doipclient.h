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
#include "diagnosticmessagehandler.h"
#include "doipgenericheaderhandler.h"

namespace figkey {

class DoIPClient{
public:
    DoIPClient();


    bool sendAliveCheckResponse();

private:
    //int _sockFd, _sockFd_udp, _connected;
    unsigned short sourceAddress{0x0E01};
    unsigned short targetAddress{0xBBBB};

    int broadcast = 1;
    
    unsigned char VINResult [17];
    unsigned char LogicalAddressResult [2];
    unsigned char EIDResult [6];
    unsigned char GIDResult [6];
    unsigned char FurtherActionReqResult;

    const std::pair<int, unsigned char*>* buildVehicleIdentificationRequest();
    void parseVIResponseInformation(unsigned char* data);
    
    int emptyMessageCounter = 0;
};

}

#endif /* DOIPCLIENT_H */

