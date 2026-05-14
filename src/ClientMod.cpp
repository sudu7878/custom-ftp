/*FileName: ClientMod.cpp*/

#include "ClientMod.hpp"
#include "CommunMod.hpp"
#include "UserHandler.hpp"
#include "api.hpp"
#include "CommunMod.hpp"


#include <functional>
#include<thread>
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

bool ClientConnected = false;

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
        perror("[ERROR] inet_pton");
        return;
    }

    if (connect(sockfd, (struct sockaddr *) &serv_address, sizeof(serv_address)) < 0){
        perror("[ERROR] Connection Error");
        
        return;
    }

    ClientConnected = true;
            
    printf("Connected to %s:%d.\n", ip, port);
}

int ClientInstance::GetPort(){
    return serv_port;
}

void TerminateConnection(ClientInstance& client){
    ClientConnected = false;
    close(client.GetFd());
        if(EnableDebug){printf("[dbg] Client connection socket closure successful.\n");}
}

void CloseConnection(ClientInstance& client){
    ClientConnected = false;
    shutdown(client.GetFd(), SHUT_RD);
        printf("Connection closed.\n");
}

int RunRecvThread(ClientInstance& client){
    while(ClientConnected){

        Packet MessagePacket;

        std::vector<uint8_t> RecvMsgHdrBuff(5);
        TemporaryPacketHeader HeaderPacket;

        int RecvBytes = 0;
        while (RecvBytes < RecvMsgHdrBuff.size()) {
            /*handling pointer arithmetic to prevent over-writing*/
            int RecvFlag = recv(client.GetFd(), 
                                RecvMsgHdrBuff.data() + RecvBytes,
                                RecvMsgHdrBuff.size() - RecvBytes, 
                                0);

            if(RecvFlag < 0){
                    if(EnableDebug){printf("[dbg] Reading incoming buffer failed. Recvflag returned: %d\n", RecvFlag);}
                perror("[ERROR] Receiving packet failed");
                printf("Terminating connection.\n");
                TerminateConnection(client);
                ProgramRunning = false;
                return -1;
            } else if (RecvFlag == 0){
                    if(EnableDebug){printf("[dbg] Recieve flag returned %d. Closing connection.\n", RecvFlag);}
                CloseConnection(client);
                ProgramRunning = false;
                ClientConnected = false;
                return 0;
                break;
            } 
            
            else {
                RecvBytes += RecvFlag;
            }
        }
        
        RecvBytes = 0; /*reset*/
        HeaderPacket = DeserializeHeaderPacket(RecvMsgHdrBuff);

        TemporaryPacketBody BodyPacket;
        
        size_t BytesToRecieve = HeaderPacket.len + sizeof(uint8_t);

        std::vector<uint8_t> RecvMsgBodyBuff(BytesToRecieve);

        while (RecvBytes < BytesToRecieve) {
            int RecvFlag = recv(client.GetFd(), 
                                RecvMsgBodyBuff.data() + RecvBytes, 
                                BytesToRecieve - RecvBytes, 
                                0);

            if(RecvFlag < 0){
                    if(EnableDebug){printf("[dbg] Reading incoming buffer failed. Recvflag returned: %d\n", RecvFlag);}
                perror("[ERROR] Receiving packet failed");
                printf("Terminating connection. Recv fd: %d\n", RecvFlag);
                TerminateConnection(client);
                ClientConnected = false;
                return -1;
            } else if(RecvFlag == 0){
                    if(EnableDebug){printf("Recieved command to close the connection.\n");}
                CloseConnection(client);
                TerminateConnection(client);
                ClientConnected = false;
                ProgramRunning = false;
                return 0;
            }else{
                RecvBytes += RecvFlag;
            }
        }
        RecvBytes = 0;
        BodyPacket = DeserializeBodyPacket(RecvMsgBodyBuff, HeaderPacket);

        MessagePacket = CombinePacket(HeaderPacket, BodyPacket);

        if (MessagePacket.PL_TYPE == MESSAGE_BROADCAST){
            printf("[BROADCAST] Server: %s", MessagePacket.PL_BODY.data());
        }

        switch (MessagePacket.PL_CTL) {
            case NO_ARG:
                break;
            case END_CONNECTION:
                TerminateConnection(client);
                ClientConnected = false;
                break;
        }
        
        printf("[SERVER]: %s\n", MessagePacket.PL_BODY.data());
        
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

    std::thread RecvThread(RunRecvThread, std::ref(NewClient));

    while(ProgramRunning){
        
        Packet MessagePacket;

        MessagePacket.PL_TYPE = MESSAGE;
        MessagePacket.PL_CTL = NO_ARG;

        std::getline(std::cin, MessagePacket.PL_BODY);

        if(!ProgramRunning || !ClientConnected){
            break;
        }

        if(MessagePacket.PL_BODY.empty()){
            continue;
        } else if(MessagePacket.PL_BODY == "/~end~/"){
                if(EnableDebug){printf("Recieved string to close connection.\n");}
            printf("ENDING CONNECTION!\n");
            RecvThread.join();
            CloseConnection(NewClient);
            return 0;
            break;
        }

        std::vector<uint8_t> MessageBuffer = SerializePacket(MessagePacket);
            //if(EnableDebug){printf("[dbg] Made the packet ready for sending... calling send() now.\n");}

        int SendFlag = send(NewClient.GetFd(), 
                        MessageBuffer.data(), 
                        MessageBuffer.size(), 
                        0);
        
        if(EnableDebug){printf("[dbg] Packet should be sent.\n");}        
        
        printf("[YOU]: %s\n", MessagePacket.PL_BODY.c_str());

        if (SendFlag < 0){
                if(EnableDebug){printf("[dbg] Sending buffer failed. Sendflag returned: %d\n", SendFlag);}
            perror("[ERROR] Packet send error");
            printf("Sending packet failed. Terminating the cinnection\n");
            break;
        };
           
    }

    CloseConnection(NewClient);

    return 0;
}