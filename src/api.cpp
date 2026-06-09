/*FileName: ConnMod.cpp*/

#include <cstddef>
#include <cstdint>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstdlib>
#include <ifaddrs.h>


#include "api.hpp"

bool EnableDebug = false;
bool RunningMode;
volatile bool ProgramRunning = true;

void HandleExit(){
    ProgramRunning = false;
}

const char* GetLANIPAddr(){
    static char ServerIPAddress[INET_ADDRSTRLEN] = {0};
    struct ifaddrs *ifaddrFirstElement, *ifaddrCursor;

    if (getifaddrs(&ifaddrFirstElement) == -1){
        perror("getifaddrs error");
        return nullptr;
    }

    for(ifaddrCursor = ifaddrFirstElement; ifaddrCursor != NULL; ifaddrCursor = ifaddrCursor->ifa_next){
        if(ifaddrCursor->ifa_addr == NULL){
            continue;
        }

        if(ifaddrCursor->ifa_addr->sa_family == AF_INET){
            struct sockaddr_in *addr = (struct sockaddr_in *)ifaddrCursor->ifa_addr;
            inet_ntop(AF_INET, &addr->sin_addr, ServerIPAddress, sizeof(ServerIPAddress));

            if(strncmp(ServerIPAddress, "127.", 4) != 0){
                freeifaddrs(ifaddrFirstElement);
                return ServerIPAddress;
            }
        } 
    }

    freeifaddrs(ifaddrFirstElement);
    return nullptr;
}

void PrintServerInfo(uint16_t port){

    printf("\n========Server Info========\n");
    printf("Local IP: 127.0.0.1:%d\n", port);
    printf("LAN IP: %s:%d\n", GetLANIPAddr(), port);
    printf("===========================\n\n");
}

/*BASE CLASS FUNCTIONS*/

BaseConnectionInstance::BaseConnectionInstance(){
    sockfd = -1;
}

bool BaseConnectionInstance::CreateSocketFd(){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("Socket creation error");
        return false;
    }
    if(EnableDebug){printf("Socket creation successful.\n");}
    return true;
}

int BaseConnectionInstance::GetFd(){
    return sockfd;
}

BaseConnectionInstance::~BaseConnectionInstance() noexcept{
    if (sockfd >= 0){
        close(sockfd);
        if(EnableDebug){printf("The socket is now closed.\n");}
    }
    close(sockfd);
}

int RecievePacket(std::vector<uint8_t> &buffer, int fd){
    int RecvBytes = 0;
    while (RecvBytes < buffer.size()) {
        /*handling pointer arithmetic to prevent over-writing*/
        int RecvFlag = recv(fd, 
                        buffer.data() + RecvBytes,
                          buffer.size() - RecvBytes, 
                      0);

        if(RecvFlag < 0){
            if(EnableDebug){printf("[dbg] Reading incoming buffer failed. Recvflag returned: %d\n", RecvFlag);}
            perror("[ERROR] Receiving packet failed");

            return -1;

        } else if (RecvFlag == 0){
            if(EnableDebug){printf("[dbg] Recieve flag returned %d.", RecvFlag);}
            return 2;
        
        } 
        else {
            RecvBytes += RecvFlag;
            }
        }
    return 0;
};



