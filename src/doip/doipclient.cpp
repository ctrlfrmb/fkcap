#include "doip/doipclient.h"
#include "doip/doipclientconfig.h"

namespace figkey {

DoIPClient::DoIPClient()
{
     auto& config = figkey::DoIPClientConfig::Instance();
     sourceAddress = config.getSourceAddress();
     targetAddress = config.getTargetAddress();

     tc.SetMessageCallback(std::bind(&DoIPClient::receiveMessage, this, std::placeholders::_1));
}

void DoIPClient::SetMessageCallback(const opensource::ctrlfrmb::HandleTCPMessage& callback) {
    diagnosticCallback = callback;
}

/*
 *Set up the connection between client and server
 */
bool DoIPClient::startTcpConnection() {
    auto& config = figkey::DoIPClientConfig::Instance();
    std::string localIP(config.getLocalIP());
    std::string serverIP(config.getServerIP());
    auto serverPort = config.getTcpPort();
    std::cout << "doip client "<< localIP<< ", doip client server " <<serverIP<<":"<<serverPort << std::endl;

    return tc.Connect(serverIP.c_str(), serverPort, localIP.c_str());
}

bool DoIPClient::startUdpConnection(){
    auto& config = figkey::DoIPClientConfig::Instance();
    std::string localIP(config.getLocalIP());
    std::string serverIP(config.getServerIP());
    auto serverPort = config.getUdpPort();
    std::cout << "doip client "<< localIP<< ", doip client server " <<serverIP<<":"<<serverPort << std::endl;

    return uc.Connect(serverIP.c_str(), serverPort, localIP.c_str());
//    _sockFd_udp = socket(AF_INET,SOCK_DGRAM, 0);
    
//    if(_sockFd_udp >= 0)
//    {
//        std::cout << "Client-UDP-Socket created successfully" << std::endl;
        
//        _serverAddr.sin_family = AF_INET;
//        _serverAddr.sin_port = htons(_serverPortNr);
//        _serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        
//        _clientAddr.sin_family = AF_INET;
//        _clientAddr.sin_port = htons(_serverPortNr);
//        _clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        
//        //binds the socket to any IP Address and the Port Number 13400
//        bind(_sockFd_udp, (struct sockaddr *)&_clientAddr, sizeof(_clientAddr));
//    }
}

/*
 * closes the client-socket
 */
void DoIPClient::closeTcpConnection(){  
    tc.Disconnect();
    //close(_sockFd);
}

void DoIPClient::closeUdpConnection(){
    uc.Disconnect();
    //close(_sockFd_udp);
}

void DoIPClient::reconnectServer(){
    closeTcpConnection();
    startTcpConnection();
}

/*
 *Build the Routing-Activation-Request for server
 */
std::vector<uint8_t> DoIPClient::buildRoutingActivationRequest() {
   std::vector<uint8_t> data(15, 0);
  
   auto& config = figkey::DoIPClientConfig::Instance();
   auto ver = config.getVersion();
   auto type  = config.getActiveType();
   //Generic Header
   data[0]= (unsigned char)(ver & 0x000000FF);  //Protocol Version
   data[1]= ~data[0];  //Inverse Protocol Version
   data[2]=0x00;  //Payload-Type
   data[3]=0x05;
   data[4]=0x00;  //Payload-Length
   data[5]=0x00;
   data[6]=0x00;
   data[7]=0x07;
   
   //Payload-Type specific message-content
   data[8] = (unsigned char)((sourceAddress >> 8) & 0xFF); //Source Address
   data[9] = (unsigned char)(sourceAddress & 0xFF);
   data[10]= (unsigned char)(type & 0x000000FF);; //Activation-Type
   data[11]=0x00; //Reserved ISO(default)
   data[12]=0x00;
   data[13]=0x00;
   data[14]=0x00;

   if (config.getUseOEMSpecific()) {
       data.resize(19);
       auto arr  = config.getAdditionalOEMSpecific();
       if (arr.size() != 4) {
           std::cerr << "doip client config additional oem specific invalid, size" << arr.size() << std::endl;
           return data;
       }
       data.insert(data.end(), arr.begin(), arr.end());
   }
   return data;
}

/*
 * Send the builded request over the tcp-connection to server
 */
bool DoIPClient::sendRoutingActivationRequest() {
    auto data = buildRoutingActivationRequest();
    return tc.Send(data);
}

/**
 * Sends a diagnostic message to the server
 * @param targetAddress     the address of the ecu which should receive the message
 * @param userData          data that will be given to the ecu
 * @param userDataLength    length of userData
 */
bool DoIPClient::sendDiagnosticMessage(const std::vector<uint8_t>& data) {
    //static unsigned short sourceAddress = getSourceAddress
    auto message = createDiagnosticMessage(sourceAddress, targetAddress, data);
    return tc.Send(message);
}

/**
 * Sends a alive check response containing the clients source address to the server
 */
bool DoIPClient::sendAliveCheckResponse() {
    int responseLength = 2;
    auto message = createGenericHeader(PayloadType::ALIVECHECKRESPONSE, responseLength);
    message[8] = (unsigned char)((sourceAddress >> 8) & 0xFF); //Source Address
    message[9] = (unsigned char)(sourceAddress & 0xFF);
    return tc.Send(message);
}

/*
 * Receive a message from server
 */
void DoIPClient::receiveMessage(std::vector<uint8_t> data) {
    if (diagnosticCallback)
        diagnosticCallback(data);

//    printf("Client received: ");
//    for(size_t i = 0; i < data.size(); i++)
//    {
//        printf("0x%02X ", _receivedData[i]);
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
}

void DoIPClient::receiveUdpMessage() {
    
//    unsigned int length = sizeof(_clientAddr);
    
//    int readedBytes;
//    readedBytes = recvfrom(_sockFd_udp, _receivedData, _maxDataSize, 0, (struct sockaddr*)&_clientAddr, &length);
    
//    if(PayloadType::VEHICLEIDENTRESPONSE == parseGenericHeader(_receivedData, readedBytes).type)
//    {
//        parseVIResponseInformation(_receivedData);
//    }
    
}

const std::pair<int,unsigned char*>* DoIPClient::buildVehicleIdentificationRequest(){
    
    std::pair <int,unsigned char*>* rareqWithLength= new std::pair<int,unsigned char*>();
    int rareqLength= 8;
    unsigned char * rareq= new unsigned char[rareqLength];

    //Generic Header
    rareq[0]=0x02;  //Protocol Version
    rareq[1]=0xFD;  //Inverse Protocol Version
    rareq[2]=0x00;  //Payload-Type
    rareq[3]=0x01;
    rareq[4]=0x00;  //Payload-Length
    rareq[5]=0x00;  
    rareq[6]=0x00;
    rareq[7]=0x00;

    rareqWithLength->first=rareqLength;
    rareqWithLength->second=rareq;

    return rareqWithLength;
    
}

void DoIPClient::sendVehicleIdentificationRequest(const char* /*address*/){
     
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
}

void DoIPClient::parseVIResponseInformation(unsigned char* data){
    
    //VIN
    int j = 0;
    for(int i = 8; i <= 24; i++)
    {      
        VINResult[j] = data[i];
        j++;
    }
    
    //Logical Adress
    j = 0;
    for(int i = 25; i <= 26; i++)
    {
        LogicalAddressResult[j] = data[i];
        j++;
    }
      
    //EID
    j = 0;
    for(int i = 27; i <= 32; i++)
    {
        EIDResult[j] = data[i];
        j++;
    }
    
    //GID
    j = 0;
    for(int i = 33; i <= 38; i++)
    {
        GIDResult[j] = data[i];
        j++;
    }
    
    //FurtherActionRequest
    FurtherActionReqResult = data[39];
    
}

void DoIPClient::displayVIResponseInformation()
{
    //output VIN
    std::cout << "VIN: ";
    for(int i = 0; i < 17 ;i++)
    {
        std::cout << (unsigned char)(int)VINResult[i];
    }
    std::cout << std::endl;
    
    //output LogicalAddress
    std::cout << "LogicalAddress: ";
    for(int i = 0; i < 2; i++)
    {
        printf("%02X", (int)LogicalAddressResult[i]);
    }
    std::cout << std::endl;
    
    //output EID
    std::cout << "EID: ";
    for(int i = 0; i < 6; i++)
    {
        printf("%02X", EIDResult[i]);
    }
    std::cout << std::endl;
    
     //output GID
    std::cout << "GID: ";
    for(int i = 0; i < 6; i++)
    {
        printf("%02X", (int)GIDResult[i]);
    }
    std::cout << std::endl;
    
    //output FurtherActionRequest
    std::cout << "FurtherActionRequest: ";
    printf("%02X", (int)FurtherActionReqResult);
    
    std::cout << std::endl;
}

}
