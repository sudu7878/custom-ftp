/*FileName: ClientMod.cpp*/

#include "ClientMod.hpp"
#include "UserHandler.hpp"
#include "api.hpp"

#include <array>
#include <stdio.h>
#include <stdbool.h>
#include <cstring>
#include <stdint.h>
#include <sys/socket.h>


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


void TerminateConnection(BaseConnectionInstance& ConnectionInstance){
    ConnectionInstance.CloseManually();
}

std::array<char, 2048> Clientmsgbuff;

//CLIENT LOOP

int StartClient(const char* ip, uint16_t port){
    printf("Attempting to connect server at %s:%d\n", ip, port);
        
    ClientInstance NewClient;
    NewClient.CreateSocketFd();
        if(EnableDebug){printf("[dbg] Client socket creation successful. Socket FD: %d\n", NewClient.GetFd());}
    NewClient.ConnectToServer(ip, port);
        if(EnableDebug){printf("[dbg] Connection successful. Socket : %d.\n", NewClient.GetFd());}

    while(ProgramRunning){
        Clientmsgbuff.fill(0);
        int RecvFlag = recv(NewClient.GetFd(), 
                            Clientmsgbuff.data(), 
                            Clientmsgbuff.size(), 
                            0);

        if (RecvFlag < 0){
                if(EnableDebug){printf("[dbg] Client recieve flag returned: %d.\n", RecvFlag);}
            perror("Error reading buffer");
        } else if(RecvFlag == 0){
            printf("Server disconnected. Run the program again to reconnect.\n");
            close(NewClient.GetFd());
                if(EnableDebug){printf("[dbg] Disconnect successful.\n");}
            break;
        }

        printf("Server: %s", Clientmsgbuff.data());

        Clientmsgbuff.fill(0);

        fgets(Clientmsgbuff.data(), 
                Clientmsgbuff.size(), 
                stdin);    /*get whatever shit user is typing*/

        int SendFlag = send(NewClient.GetFd(), 
                            Clientmsgbuff.data(), 
                            strlen(Clientmsgbuff.data()), 
                            0);
                            
        printf("You: %s", Clientmsgbuff.data());

        if (SendFlag < 0){
                if(EnableDebug){printf("[dbg] Client send flag returned: %d.\n", SendFlag);}
            perror("Error writing buffer");
        };
           
        if(strcmp("/~end~/", Clientmsgbuff.data()) == 0){
            printf("Ending connection.\n");
                if(EnableDebug){printf("[dbg] Recieved end command. Disconnect successful.\n");}
            break;
        }
    }
    close(NewClient.GetFd());
        if(EnableDebug){printf("[dbg] Client socket closure successful.\n");}

    return 0;
}