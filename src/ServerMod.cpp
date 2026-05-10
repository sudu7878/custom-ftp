/*FileName: ServerMod.cpp*/

#include "ServerMod.hpp"
#include "CommunMod.hpp"

#include <array>
#include <cstdint>
#include <cstring>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string>
#include <unistd.h>
#include <vector>
#include "UserHandler.hpp"
#include "api.hpp"
#include <iostream>


/*SERVER CLASS FUNCTIONS*/

int CommunicationSocketFd = 0;

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
        if(EnableDebug){ printf("[dbg] Socket bound to 0.0.0.0 at port %d.\n", serv_port);}
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

void StopServer(ServerInstance& server){
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

        /*Main loop*/
        while(ProgramRunning){
            
           NewServer.AcceptClient();
                if(EnableDebug){printf("[dbg] Server accepted a client.\n");}
        
           BroadcastServerMsg("Accepted a client!\n");
                if(EnableDebug){printf("[dbg] Broadcast successful.\n");}


           Packet MessagePacket;
           MessagePacket.PL_TYPE = MESSAGE;
           MessagePacket.PL_CTL = NO_ARG;

           std::getline(std::cin, MessagePacket.PL_BODY);
           std::vector<uint8_t> MessageBuffer = SerializePacket(MessagePacket);

           int SendFlag = send(CommunicationSocketFd, 
                                MessageBuffer.data(), 
                                MessageBuffer.size(), 
                                0);

           printf("You: %s", MessagePacket.PL_BODY.c_str());

           if (SendFlag < 0){
                if(EnableDebug){printf("[dbg] Sending buffer failed. Sendflag returned: %d\n", SendFlag);}
            perror("Sending packet failed");
           };
           
        }

        StopServer(NewServer);

        return 0;
}