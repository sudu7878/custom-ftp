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


    /*LOGIC FOR RECIEVING PACKET*/    
        Packet MessagePacket;

        std::vector<uint8_t> RecvMsgHdrBuff(5);
        TemporaryPacketHeader HeaderPacket;

        int RecieveStatusHdr = RecievePacket(RecvMsgHdrBuff, client.GetFd());
        if (RecieveStatusHdr == 2){
            if(EnableDebug){printf("[dbg] Closing connection.\n");}
            CloseConnection(client);
            ProgramRunning = false;
            ClientConnected = false;
            break;
        } else if(RecieveStatusHdr < 0){
            printf("Terminating connection.\n");
            TerminateConnection(client);
            ProgramRunning = false;
            break;
        }
        
        /*to recieve body packet based on header packet attributes*/
        HeaderPacket = DeserializeHeaderPacket(RecvMsgHdrBuff);

        TemporaryPacketBody BodyPacket;
        
        size_t BytesToRecieve = HeaderPacket.len + sizeof(uint8_t);

        std::vector<uint8_t> RecvMsgBodyBuff(BytesToRecieve);

        int RecieveStatusBdy = RecievePacket(RecvMsgBodyBuff, client.GetFd());
        if (RecieveStatusBdy == 2){
                if(EnableDebug){printf("[dbg] Closing connection.\n");}
            CloseConnection(client);
            ProgramRunning = false;
            ClientConnected = false;
            break;
        } else if(RecieveStatusBdy < 0){
                if(EnableDebug){printf("[dbg] Receiving packet failed.\n");}
            printf("Terminating connection.\n");
            TerminateConnection(client);
            ProgramRunning = false;
            break;
        }

        BodyPacket = DeserializeBodyPacket(RecvMsgBodyBuff, HeaderPacket);

        /*Combining the header + body packet to form message packet*/
        MessagePacket = CombinePacket(HeaderPacket, BodyPacket);    //recieving complete

    /*PACKET PASRSING LOGIC*/

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
    if (EnableDebug){printf("[dbg] Attempting to connect server at %s:%d\n", ip, port);};
    
    /*INITIALIZE CLIENT*/

    ClientInstance NewClient;
    NewClient.CreateSocketFd();
        if(EnableDebug){printf("[dbg] Client socket creation successful. Socket FD: %d\n", NewClient.GetFd());}
    NewClient.ConnectToServer(ip, port);
        if(EnableDebug){printf("[dbg] Connection successful. Socket : %d.\n", NewClient.GetFd());}

    std::thread RecvThread(RunRecvThread, std::ref(NewClient));

    /*START MAIN PROGRAM LOOP*/
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
                if(EnableDebug){printf("[dbg] Recieved string to close connection.\n");}
            printf("ENDING CONNECTION!\n");
            RecvThread.join();
            CloseConnection(NewClient);
            return 0;
            break;
        }

        /*SENDING PACKET LOGIC*/

        std::vector<uint8_t> MessageBuffer = SerializePacket(MessagePacket);
            //if(EnableDebug){printf("[dbg] Made the packet ready for sending... calling send() now.\n");}

        int SendFlag = SendPacket(MessageBuffer, NewClient.GetFd());
        
        if(SendFlag == 0){
                if(EnableDebug){printf(" [dbg] The packet send was successful.\n");}
        } else if (SendFlag < 0){
            printf("Sending packet failed. Terminating the cinnection\n");
            break;
        };
        
        printf("[YOU]: %s\n", MessagePacket.PL_BODY.c_str());
           
    }

    CloseConnection(NewClient);

    return 0;
}


