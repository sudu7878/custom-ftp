/*FileName: api.hpp*/

#pragma once

#ifndef API_MODULE_HPP
#define API_MODULE_HPP


#include <stdbool.h>
#include <stdint.h>

#include <sys/errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

#include <ifaddrs.h>

extern bool EnableDebug;
extern volatile bool ProgramRunning;
extern bool RunningMode;                       /*1 for server. 0 for client */

void HandleExit(int sig);

const char* GetLANIPAddr();

class BaseConnectionInstance{
    protected:
        int sockfd;
    public:
        BaseConnectionInstance();
        bool CreateSocketFd();
        int GetFd();
        virtual ~BaseConnectionInstance() noexcept;

};

void PrintServerInfo(uint16_t port);

void TerminateConnection(BaseConnectionInstance& ConnectionInstance);

#endif  /*API_HPP*/