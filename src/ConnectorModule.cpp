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
#include <signal.h>

#include <headers/ConnectorMoudle.hpp>



bool RunningMode;                       /*1 for server. 0 for client */
bool EnableDebug = false;
volatile bool ProgramRunning = true;

void HandleExit(int sig){
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

void TerminateConnection(BaseConnectionInstance& ConnectionInstance){
    ConnectionInstance.CloseManually();
}

class BaseConnectionInstance{
    protected:
        int sockfd;

    public:
        BaseConnectionInstance(){
            sockfd = -1;
        }

        bool CreateSocketFd(){
           sockfd = socket(AF_INET, SOCK_STREAM, 0);
           if(sockfd < 0){
               perror("Socket creation error");
               return false;
           }
           if(EnableDebug){printf("Socket creation successful.\n");}
           return true;
        }

        int GetFd(){
            return sockfd;
        }

        void CloseManually(){
            if(sockfd >= 0){
                close(sockfd);
                sockfd = -1;
                if(EnableDebug){printf("The socket has been closed manually.\n");}
            } else {
                printf("Close Connection Error: Incorrect file descriptor or the connection is already closed.\n");
            }
        }

        virtual ~BaseConnectionInstance() noexcept{
            if (sockfd >= 0){
                close(sockfd);
                if(EnableDebug){printf("The socket is now closed.\n");}
            }
        }

};

class ServerInstance : public BaseConnectionInstance{
    private:
        uint16_t serv_port = 8000;

    public:
        int GetPort(){
            return serv_port;
        }

        void StartListening(){
            if (listen(sockfd, SOMAXCONN) < 0){
                printf("Error: Couldn't listen.\n");
            } else{
                PrintServerInfo(serv_port);
                printf("Listening on port %d...\n", serv_port);
            }
        }

        void BindSocketToServer(){
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

        int AcceptClient(){
            struct sockaddr_in cli_addr{};
            socklen_t cli_addr_len = sizeof(cli_addr);

            int NewSockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_addr_len);

            if (NewSockfd < 0){
                perror("Unable to accept");
                return -1;
            }

            printf("Client is now connected.\n");
            return NewSockfd;
        }
        
        ~ServerInstance() noexcept override{}
};

class ClientInstance : public BaseConnectionInstance{
    private:
        uint16_t serv_port;

    public:
        void ConnectToServer(const char* ip, uint16_t port){
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

        int GetPort(){
            return serv_port;
        }

        ~ClientInstance()noexcept override{}
};

int main(int argc, char *argv[]){
    if(argc < 2){
        printf("Needs arguments.\n");
        return -1;
    } 

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-dbg") == 0){
            EnableDebug = true;
            printf("[DEBUG MODE ON]\n");
        }
    }

    if(strcmp(argv[1], "server") == 0){
        RunningMode = true;
        printf("Program running in server mode.\n");

    } else if(strcmp(argv[1], "client") == 0){
        RunningMode = false;
        if(argc < 4){
            printf("Usage: ./program client <ip> <port>\n");
            return -1;
        }
        printf("Program running in client mode.\n");

    } else {
        printf("Invalid arguments.\n");
        return -1;
    }
    
    //SERVER MODE
    if(RunningMode){
        ServerInstance NewServer;
        NewServer.CreateSocketFd();
        NewServer.BindSocketToServer();
        NewServer.StartListening();
        while(ProgramRunning){
            NewServer.AcceptClient();
        }
    }
    

    //CLIENT MODE
    if (!RunningMode){
        const char* ip = argv[2];
        uint16_t port = atoi(argv[3]);

        printf("Attempting to connect server at %s:%d\n", ip, port);
        
        ClientInstance NewClient;
        NewClient.CreateSocketFd();
        NewClient.ConnectToServer(ip, port);
        while(ProgramRunning){

        }
    }
    

    return 0;
}