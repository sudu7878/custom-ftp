/*FileName: ServerMod.cpp*/

#include "ServerMod.hpp"
#include "CommunMod.hpp"


#include <cstdint>
#include <cstring>
#include <functional>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string>
#include <unistd.h>
#include <vector>
#include "UserHandler.hpp"
#include "api.hpp"
#include <iostream>

#include <thread>


/*SERVER CLASS FUNCTIONS*/

int CommunicationSocketFd = 0;
bool ServerConnected = false;

int ServerInstance::GetPort(){
    return serv_port;
}

void ServerInstance::StartListening(){
    if (listen(sockfd, 5) < 0){
        printf("Error: Couldn't listen.\n");
    }else{
        PrintServerInfo(serv_port);
        printf("Listening on port %d...\n", serv_port);
    }
}

void ServerInstance::BindSocketToServer(){
    struct sockaddr_in serv_address{};
    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(serv_port);
    serv_address.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sockfd, (struct sockaddr *) &serv_address, sizeof(serv_address)) < 0){
        perror("Binding Error");
        return;
    }
        if(EnableDebug){ printf("[dbg] Socket bound to port %d.\n", serv_port);}
}

int ClientCount = 0;

int ServerInstance::AcceptClient(){
  
    struct sockaddr_in cli_addr{};
    socklen_t cli_addr_len = sizeof(cli_addr);

    int NewSockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_addr_len);

        if(EnableDebug){printf("[dbg] Server accepted connection. Listening socket FD: %d.\n", NewSockfd);}

    if (NewSockfd < 0){
            if(EnableDebug){printf("[dbg] Server failed to accept client. Listening socket FD: %d.\n", NewSockfd);}
        perror("Unable to accept client");
        return -1;
    }
    ClientCount++;
    printf("Client is now connected. Number of clients now: %d\n", ClientCount);

    ServerConnected = true;

    CommunicationSocketFd = NewSockfd;
   
    return NewSockfd;
}

void BroadcastServerMsg(const std::string& message){
    Packet BroadcastPacket;

    BroadcastPacket.PL_BODY = message;
    BroadcastPacket.PL_TYPE = MESSAGE_BROADCAST;
    
    std::vector<uint8_t> BroadcastBuffer = SerializePacket(BroadcastPacket);

    int SendFlag = send(CommunicationSocketFd, 
                        BroadcastBuffer.data(), 
                        BroadcastBuffer.size(), 
                        0);

    if (SendFlag < 0){
        perror("CRITICAL-Error writing server message buffer");
            if(EnableDebug){printf("[dbg] Broadcast FAIL. Server send flag: %d\n", SendFlag);}
    };
        if(EnableDebug){printf("[dbg] Broadcast successful.\n");}
}

int RunRecvThread(ServerInstance& server){
        //if(EnableDebug){printf("[dbg] Recieve thread for server is running. Server connected = %d\n", ServerConnected);}
    while(ServerConnected == true){
        //if(EnableDebug){printf("[dbg] INSIDE THE RECV LOOP NOW.\n");}
        Packet MessagePacket;

        std::vector<uint8_t> RecvMsgHdrBuff(5);
        TemporaryPacketHeader HeaderPacket;

        int RecvBytes = 0;
        while (RecvBytes < RecvMsgHdrBuff.size()) {
            /*handling pointer arithmetic to prevent over-writing*/
            int RecvFlag = recv(CommunicationSocketFd, 
                                RecvMsgHdrBuff.data() + RecvBytes,
                                RecvMsgHdrBuff.size() - RecvBytes, 
                                0);
            
          

            if(RecvFlag < 0){
                    if(EnableDebug){printf("[dbg] Reading incoming buffer failed. Recvflag returned: %d\n", RecvFlag);}
                perror("Receiving packet failed");
                return -1;
            } else {
                RecvBytes += RecvFlag;
            }
        }
        
        RecvBytes = 0; /*reset*/
        HeaderPacket = DeserializeHeaderPacket(RecvMsgHdrBuff);
        if(EnableDebug){printf("[dbg] Header Type: %d | Header Len: %u\n", HeaderPacket.type, HeaderPacket.len);}

        TemporaryPacketBody BodyPacket;
        
        size_t BytesToRecieve = HeaderPacket.len + sizeof(uint8_t);

        std::vector<uint8_t> RecvMsgBodyBuff(BytesToRecieve);

        while (RecvBytes < BytesToRecieve) {
            int RecvFlag = recv(CommunicationSocketFd, 
                                RecvMsgBodyBuff.data() + RecvBytes, 
                                BytesToRecieve - RecvBytes, 
                                0);

            if(RecvFlag < 0){
                    if(EnableDebug){printf("[dbg] Reading incoming buffer failed. Recvflag returned: %d\n", RecvFlag);}
                perror("Receiving packet failed\n");
                
                break;
                //TODO: Add a logic for clients leaving when RecvFlag = 0
            } else {
                RecvBytes += RecvFlag;
            }
        }
        RecvBytes = 0;
        BodyPacket = DeserializeBodyPacket(RecvMsgBodyBuff, HeaderPacket);

        MessagePacket = CombinePacket(HeaderPacket, BodyPacket);
         printf("[CLIENT]: %s\n", MessagePacket.PL_BODY.data());
        if (MessagePacket.PL_TYPE == MESSAGE_BROADCAST){
            printf("[BROADCAST] Server: %s", MessagePacket.PL_BODY.data());
        }

        switch (MessagePacket.PL_CTL) {
            case NO_ARG:
                break;
            case END_CONNECTION:
                printf("[ACTION] CLIENT LEFT.\n");
                break;
        }
        
       
        
    }   
    return 0;
}

void StopServer(ServerInstance& server){
    ServerConnected = false;
    close(CommunicationSocketFd);
            if(EnableDebug){printf("[dbg] Server listening socket closure successful.\n");}

    close(server.GetFd());
            if(EnableDebug){printf("[dbg] Server connection socket closure successful.\n");}
}

void LockDoor(ServerInstance& server){
     close(server.GetFd());
            if(EnableDebug){printf("[dbg] Server connection socket closure successful.\n");}
}

//SERVER LOOP

int StartServer(){
    ServerInstance NewServer;
        NewServer.CreateSocketFd();
            if(EnableDebug){printf("[dbg] Server socket creation successful. Socket FD: %d\n", NewServer.GetFd());}
        NewServer.BindSocketToServer();
            if(EnableDebug){printf("[dbg] Binding socekt to address successful.\n");}
        NewServer.StartListening();
            if(EnableDebug){printf("[dbg] Server reached the listening state successfully.\n");}
        
        NewServer.AcceptClient();
                if(EnableDebug){printf("[dbg] Server accepted a client.\n");}
        ServerConnected = true;
                

            
        std::thread RecvThread(RunRecvThread, std::ref(NewServer));
           

        BroadcastServerMsg("Accepted a client!\n");
                if(EnableDebug){printf("[dbg] Broadcast successful.\n");}


        /*MAIN LOOP*/
        while(ProgramRunning){
            
           
           Packet MessagePacket;
           MessagePacket.PL_TYPE = MESSAGE;
           MessagePacket.PL_CTL = NO_ARG;
              

            std::getline(std::cin, MessagePacket.PL_BODY);
          
           std::vector<uint8_t> MessageBuffer = SerializePacket(MessagePacket);
                if(EnableDebug){printf("[dbg] Made the packet ready for sending... calling send() now.\n");}

           int SendFlag = send(CommunicationSocketFd, 
                                MessageBuffer.data(), 
                                MessageBuffer.size(), 
                                0);
            
            if(EnableDebug){printf("[dbg] Packet shud be sent.\n");}

            printf("[YOU]: %s\n", MessagePacket.PL_BODY.c_str());

            

           if (SendFlag < 0){
                if(EnableDebug){printf("[dbg] Sending buffer failed. Sendflag returned: %d\n", SendFlag);}
            perror("Sending packet failed");
           };
           
        }

        ServerConnected  = false;
        StopServer(NewServer);

        return 0;
}