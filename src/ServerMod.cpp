/*FileName: ServerMod.cpp*/

#include "ServerMod.hpp"

#include <array>
#include <cstring>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string>
#include <unistd.h>
#include "UserHandler.hpp"
#include "api.hpp"


/*SERVER CLASS FUNCTIONS*/

int ListeningSocketFd = 0;

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
    if(EnableDebug){ printf("Socket bound to 0.0.0.0 at port %d.\n", serv_port);}
}

int ClientCount = 0;

int ServerInstance::AcceptClient(){
  
    struct sockaddr_in cli_addr{};
    socklen_t cli_addr_len = sizeof(cli_addr);

    int NewSockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_addr_len);

    if (NewSockfd < 0){
        perror("Unable to accept client");
        return -1;
    }
    ClientCount++;
    printf("Client is now connected. Number of clients now: %d\n", ClientCount);

    ListeningSocketFd = NewSockfd;
   
    return NewSockfd;
}

std::array<char, 2048> Servmsgbuff; /*Main message memory buffer*/


void BroadcastServerMsg(const std::string& message){
    std::array<char, 2048> BroadcastMsgBuff;

    BroadcastMsgBuff.fill(0);

    memcpy(BroadcastMsgBuff.data(), message.data(), message.size());

    int SendFlag = send(ListeningSocketFd, BroadcastMsgBuff.data(), strlen(BroadcastMsgBuff.data()), 0);

    if (SendFlag < 0){
        perror("CRITICAL-Error writing server message buffer");
        if(EnableDebug){printf("[dbg] Broadcast FAIL.\n");}
    };

    BroadcastMsgBuff.fill(0);

    if(EnableDebug){printf("[dbg] Broadcast successful.\n");}
}

int StartServer(){
    ServerInstance NewServer;
        NewServer.CreateSocketFd();
        NewServer.BindSocketToServer();
        NewServer.StartListening();

        /*Main loop*/
        while(ProgramRunning){
            
           NewServer.AcceptClient();

           BroadcastServerMsg("Accepted a client!\n");

           Servmsgbuff.fill(0);
           int RecvFlag = recv(ListeningSocketFd, Servmsgbuff.data(), Servmsgbuff.size(), 0);

           if (RecvFlag < 0){
            perror("Error reading buffer");
           } else if (RecvFlag == 0){
            printf("Client disconected\n");
            ClientCount--;
            printf("Number of clients now: %d.\n", ClientCount);
           }

           printf("Client: %s", Servmsgbuff.data());

           Servmsgbuff.fill(0);

           fgets(Servmsgbuff.data(), Servmsgbuff.size(), stdin);    /*get whatever shit user is typing*/

           int SendFlag = send(ListeningSocketFd, Servmsgbuff.data(), strlen(Servmsgbuff.data()), 0);
           printf("You: %s", Servmsgbuff.data());

           if (SendFlag < 0){
            perror("Error writing buffer");
           };
           
           if(strcmp("/~end~/", Servmsgbuff.data()) == 0){
                printf("Ending connection.\n");
                break;
           }
        }
        close(NewServer.GetFd());
        close(ListeningSocketFd);

        return 0;
}