/*FileName: ClientMod.cpp*/

#include "ClientMod.hpp"
#include "UserHandler.hpp"
#include "api.hpp"
#include "CommunMod.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdio.h>
#include <stdbool.h>
#include <cstring>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>


/*CLIENT CLASS FUNCTIONS*/

void ClientInstance::ConnectToServer(const char* ip, uint16_t port){
    struct sockaddr_in serv_address{};
    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(port);
    serv_port = port;
    
    int res = inet_pton(AF_INET, ip, &serv_address.sin_addr);
            
    if(res == 0){
        printf("Invalid IP Address format.\n");
    } else if (res < 0){
        perror("inet_pton");
        return;
    }

    if (connect(sockfd, (struct sockaddr *) &serv_address, sizeof(serv_address)) < 0){
        perror("Connection Error");
        
        return;
    }
            
    printf("Connected to %s:%d.\n", ip, port);
}

int ClientInstance::GetPort(){
    return serv_port;
}

void TerminateConnection(ClientInstance& client){
    close(client.GetFd());
        if(EnableDebug){printf("[dbg] Client connection socket closure successful.\n");}
}

int RunRecvThread(ClientInstance& client){
    while(ProgramRunning){

        Packet MessagePacket;

        std::vector<uint8_t> RecvMsgHdrBuff(5);
        TemporaryPacketHeader HeaderPacket;

        int RecvBytes = 0;
        while (RecvBytes < RecvMsgHdrBuff.size()) {
            int RecvFlag = recv(client.GetFd(), RecvMsgHdrBuff.data(), RecvMsgHdrBuff.size(), 0);

            if(RecvFlag < 0){
                    if(EnableDebug){printf("[dbg] Reading incoming buffer failed. Recvflag returned: %d\n", RecvFlag);}
                perror("Receiving packet failed. Terminating connection.\n");
                break;
            } else {
                RecvBytes += RecvFlag;
            }
        }
        
        RecvBytes = 0; /*reset*/
        HeaderPacket = DeserializeHeaderPacket(RecvMsgHdrBuff);

        TemporaryPacketBody BodyPacket;
        
        size_t BytesToRecieve = HeaderPacket.len + sizeof(uint8_t);

        std::vector<uint8_t> RecvMsgBodyBuff;

        while (RecvBytes < BytesToRecieve) {
            int RecvFlag = recv(client.GetFd(), RecvMsgBodyBuff.data(), BytesToRecieve, 0);

            if(RecvFlag < 0){
                    if(EnableDebug){printf("[dbg] Reading incoming buffer failed. Recvflag returned: %d\n", RecvFlag);}
                perror("Receiving packet failed. Terminating connection.\n");
                TerminateConnection(client);
                return -1;
                break;
            } else {
                RecvBytes += RecvFlag;
            }
        }
        RecvBytes = 0;
        BodyPacket = DeserializeBodyPacket(RecvMsgBodyBuff);

        MessagePacket = CombinePacket(HeaderPacket, BodyPacket);

        if (MessagePacket.PL_TYPE == MESSAGE_BROADCAST){
            printf("[BROADCAST] Server: %s", MessagePacket.PL_BODY.data());
        }

        switch (MessagePacket.PL_CTL) {
            case NO_ARG:
                break;
            case END_CONNECTION:
                TerminateConnection(client);
                break;
        }
        
        printf("Server: %s", MessagePacket.PL_BODY.data());
        
    }   
    return 0;
}

//CLIENT LOOP

int StartClient(const char* ip, uint16_t port){
    printf("Attempting to connect server at %s:%d\n", ip, port);
        
    ClientInstance NewClient;
    NewClient.CreateSocketFd();
        if(EnableDebug){printf("[dbg] Client socket creation successful. Socket FD: %d\n", NewClient.GetFd());}
    NewClient.ConnectToServer(ip, port);
        if(EnableDebug){printf("[dbg] Connection successful. Socket : %d.\n", NewClient.GetFd());}

    
    while(ProgramRunning){
        Packet MessagePacket;

        MessagePacket.PL_TYPE = MESSAGE;
        MessagePacket.PL_CTL = NO_ARG;

        std::getline(std::cin, MessagePacket.PL_BODY);

        if(MessagePacket.PL_BODY.empty()){
            continue;
        }

        std::vector<uint8_t> MessageBuffer = SerializePacket(MessagePacket);

        int SendFlag = send(NewClient.GetFd(), 
                        MessageBuffer.data(), 
                        MessageBuffer.size(), 
                        0);
                            

        if (SendFlag < 0){
                if(EnableDebug){printf("[dbg] Sending buffer failed. Sendflag returned: %d\n", SendFlag);}
            perror("Sending packet failed. Terminating connection.\n");
            break;
        };
           
    }

    TerminateConnection(NewClient);

    return 0;
}