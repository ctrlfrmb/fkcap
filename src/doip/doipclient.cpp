#include "doip/doipclient.h"
#include "doip/doipclientconfig.h"

namespace figkey {

DoIPClient::DoIPClient()
{
     auto& config = figkey::DoIPClientConfig::Instance();
     sourceAddress = config.getSourceAddress();
     targetAddress = config.getTargetAddress();
}


/**
 * Sends a alive check response containing the clients source address to the server
 */
bool DoIPClient::sendAliveCheckResponse() {
    //int responseLength = 2;
    //auto message = createGenericHeader(PayloadType::ALIVECHECKRESPONSE, responseLength);
    //message[8] = (unsigned char)((sourceAddress >> 8) & 0xFF); //Source Address
    //message[9] = (unsigned char)(sourceAddress & 0xFF);
    return true;
}

/*
 * Receive a message from server
 */
//void DoIPClient::receiveMessage(std::vector<uint8_t> data) {
//    printf("doip client received: ");
//    for(size_t i = 0; i < data.size(); i++)
//    {
//        printf("%02X ", data[i]);
//    }
//    printf("\n ");

//    GenericHeaderAction action = parseGenericHeader(_receivedData, readedBytes);

//    if(action.type == PayloadType::DIAGNOSTICPOSITIVEACK || action.type == PayloadType::DIAGNOSTICNEGATIVEACK) {
//        switch(action.type) {
//            case PayloadType::DIAGNOSTICPOSITIVEACK: {
//                std::cout << "Client received diagnostic message positive ack with code: ";
//                printf("0x%02X ", _receivedData[12]);
//                break;
//            }
//            case PayloadType::DIAGNOSTICNEGATIVEACK: {
//                std::cout << "Client received diagnostic message negative ack with code: ";
//                printf("0x%02X ", _receivedData[12]);
//                break;
//            }
//            default: {
//                std::cerr << "not handled payload type occured in receiveMessage()" << std::endl;
//                break;
//            }
//        }
//        std::cout << std::endl;
//    }
//}

//void DoIPClient::receiveUdpMessage() {
    
//    unsigned int length = sizeof(_clientAddr);
    
//    int readedBytes;
//    readedBytes = recvfrom(_sockFd_udp, _receivedData, _maxDataSize, 0, (struct sockaddr*)&_clientAddr, &length);
    
//    if(PayloadType::VEHICLEIDENTRESPONSE == parseGenericHeader(_receivedData, readedBytes).type)
//    {
//        parseVIResponseInformation(_receivedData);
//    }
    
//}

//const std::pair<int,unsigned char*>* DoIPClient::buildVehicleIdentificationRequest(){
    
//    std::pair <int,unsigned char*>* rareqWithLength= new std::pair<int,unsigned char*>();
//    int rareqLength= 8;
//    unsigned char * rareq= new unsigned char[rareqLength];

//    //Generic Header
//    rareq[0]=0x02;  //Protocol Version
//    rareq[1]=0xFD;  //Inverse Protocol Version
//    rareq[2]=0x00;  //Payload-Type
//    rareq[3]=0x01;
//    rareq[4]=0x00;  //Payload-Length
//    rareq[5]=0x00;
//    rareq[6]=0x00;
//    rareq[7]=0x00;

//    rareqWithLength->first=rareqLength;
//    rareqWithLength->second=rareq;

//    return rareqWithLength;
    
//}

//void DoIPClient::sendVehicleIdentificationRequest(const char* /*address*/){
     
//    int setAddressError = inet_aton(address,&(_serverAddr.sin_addr));
    
//    if(setAddressError != 0)
//    {
//        std::cout <<"Address set succesfully"<<std::endl;
//    }
//    else
//    {
//        std::cout << "Could not set Address. Try again" << std::endl;
//    }
    
//    int socketError = setsockopt(_sockFd_udp, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast) );
         
//    if(socketError == 0)
//    {
//        std::cout << "Broadcast Option set successfully" << std::endl;
//    }
      
//    const std::pair <int,unsigned char*>* rareqWithLength=buildVehicleIdentificationRequest();
    
//    int sendError = sendto(_sockFd_udp, rareqWithLength->second,rareqWithLength->first, 0, (struct sockaddr *) &_serverAddr, sizeof(_serverAddr));
    
//    if(sendError > 0)
//    {
//        std::cout << "Sending Vehicle Identification Request" << std::endl;
//    }
//}

//void DoIPClient::parseVIResponseInformation(unsigned char* data){
    
//    //VIN
//    int j = 0;
//    for(int i = 8; i <= 24; i++)
//    {
//        VINResult[j] = data[i];
//        j++;
//    }
    
//    //Logical Adress
//    j = 0;
//    for(int i = 25; i <= 26; i++)
//    {
//        LogicalAddressResult[j] = data[i];
//        j++;
//    }
      
//    //EID
//    j = 0;
//    for(int i = 27; i <= 32; i++)
//    {
//        EIDResult[j] = data[i];
//        j++;
//    }
    
//    //GID
//    j = 0;
//    for(int i = 33; i <= 38; i++)
//    {
//        GIDResult[j] = data[i];
//        j++;
//    }
    
//    //FurtherActionRequest
//    FurtherActionReqResult = data[39];
    
//}

//void DoIPClient::displayVIResponseInformation()
//{
//    //output VIN
//    std::cout << "VIN: ";
//    for(int i = 0; i < 17 ;i++)
//    {
//        std::cout << (unsigned char)(int)VINResult[i];
//    }
//    std::cout << std::endl;
    
//    //output LogicalAddress
//    std::cout << "LogicalAddress: ";
//    for(int i = 0; i < 2; i++)
//    {
//        printf("%02X", (int)LogicalAddressResult[i]);
//    }
//    std::cout << std::endl;
    
//    //output EID
//    std::cout << "EID: ";
//    for(int i = 0; i < 6; i++)
//    {
//        printf("%02X", EIDResult[i]);
//    }
//    std::cout << std::endl;
    
//     //output GID
//    std::cout << "GID: ";
//    for(int i = 0; i < 6; i++)
//    {
//        printf("%02X", (int)GIDResult[i]);
//    }
//    std::cout << std::endl;
    
//    //output FurtherActionRequest
//    std::cout << "FurtherActionRequest: ";
//    printf("%02X", (int)FurtherActionReqResult);
    
//    std::cout << std::endl;
//}

}
