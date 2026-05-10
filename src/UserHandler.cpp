/*FileName: UserHandler.cpp*/

#include "api.hpp"

#include <cstdlib>


#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "UserHandler.hpp"


int main(int argc, char *argv[]){
    if(argc < 2){
        printf("Needs arguments.\n");
        return -1;
    }

    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-dbg") == 0){
            EnableDebug = true;
            printf("===DEBUG MODE ON: You may see overwhelming info===\n");
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
    if(RunningMode){StartServer();} 
    //CLIENT MODE
    if (!RunningMode){StartClient(argv[2], atoi(argv[3]));}

    return 0;
}