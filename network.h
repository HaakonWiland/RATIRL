#pragma once //Ensure header file is only included once when compilation 

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") //Link with winsock 2 lib

class NetworkClient {
    public:
    NetworkClient(const std::string& serverIp, int serverPort);
    ~NetworkClient();
    bool connectToServer();
    bool sendData(const std::string& data); //Consider having this as a const char*

    private:
    std::string serverIp;
    int serverPort;
    SOCKET sock; 
};
