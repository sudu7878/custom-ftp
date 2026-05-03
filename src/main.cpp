#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

class Connector{
    private:
        uint16_t port = 8000;
        int sockfd;

    public:
        Connector(){
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


       ~Connector(){
            if(sockfd >= 0){
                close(sockfd);
                printf("Socket closed.\n");
        }
    }
};

int main(){
    Connector NewConnection;
    

    return 0;
}

