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


bool RunningMode; //1 for server. 0 for client

void PrintServerInfo(uint16_t port){
    char hostbuffer[256];

    if (gethostname(hostbuffer, sizeof(hostbuffer)) < 0){
        perror("gethostname");
        return;
    }

    struct hostent* host_entry = gethostbyname(hostbuffer);
    if(!host_entry){
        perror("gethostbyname");
        return;
    }

    char* ip = inet_ntoa(*((struct in_addr*) host_entry->h_addr));

    printf("\n========Server Info========\n");
    printf("Local IP: 127.0.0.1:%d\n", port);
    printf("LAN IP: %s:%d\n", ip, port);
    printf("===========================\n\n");
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
               perror("Socket");
               return false;
           }
           printf("Socket creation successful.\n");
           return true;
        }

        int GetFd(){
            return sockfd;
        }

        virtual ~BaseConnectionInstance() noexcept{
            if (sockfd >= 0){
                close(sockfd);
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

        int AcceptClient(){
            struct sockaddr_in cli_addr{};
            socklen_t cli_addr_len = sizeof(cli_addr);

            int NewSockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_addr_len);

            if (NewSockfd < 0){
                perror("accept");
                return -1;
            }

            printf("Client is now connected.\n");
            return NewSockfd;
        }

        void BindSocketToServer(){
            struct sockaddr_in serv_address{};
            serv_address.sin_family = AF_INET;
            serv_address.sin_port = htons(serv_port);
            serv_address.sin_addr.s_addr = htonl(INADDR_ANY);

            int opt = 1;
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            if (bind(sockfd, (struct sockaddr *) &serv_address, sizeof(serv_address)) < 0){
                perror("bind");
                return;
            }
            
            printf("Socket bound to 0.0.0.0 at port %d.\n", serv_port);
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

            if (inet_pton(AF_INET, ip, &serv_address.sin_addr) <= 0){
                perror("inet_pton");
                return;
            };

            if (connect(sockfd, (struct sockaddr *) &serv_address, sizeof(serv_address)) < 0){
                perror("connect");
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

    if(strcmp(argv[1], "server") == 0){
        RunningMode = true;
        printf("Program running in server mode.\n");
    } else if(strcmp(argv[1], "client") == 0){
        RunningMode = false;
        printf("Program running in client mode.\n");
    } else {
        printf("Invalid arguments.\n");
    }

    //SERVER MODE


    //CLIENT MODE

    return 0;
}

