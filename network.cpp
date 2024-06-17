#include "network.h"
#include <iostream>

NetworkClient::NetworkClient(const std::string& _serverIp, int _serverPort)
    : serverIp(_serverIp), serverPort(_serverPort), sock(INVALID_SOCKET) {

        WSADATA wsaData;
        // Using winsock version 2.2
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed! \n";
        }
    }

NetworkClient::~NetworkClient() {

    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
    WSACleanup();
}

bool NetworkClient::connectToServer() {
    // Creating a socket with AF_INET(IPv4) and sock_stream (as the server)
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Error creating socket! \n";
        return false;
    }

    // Server address structure 
    sockaddr_in serverAddr;
    // Address family for the transport address set to AF_INET
    serverAddr.sin_family = AF_INET;
    // Portnumber for the server, and convert from HOST byte order to NETWORK byte order
    serverAddr.sin_port = htons(serverPort);
    // Converts server Ip from text to network binary, and stores it in serverAddr.sin_addr
    inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);

    //  reinterpret_cast<const sockaddr*> (&serverAddr) typecasts the memory address of serverAddr to a "const sockaddr pointer".  
    if(connect(sock, reinterpret_cast<const sockaddr*> (&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server. \n";
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }
    return true;
}