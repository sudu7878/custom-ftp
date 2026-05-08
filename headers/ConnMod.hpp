#pragma once

#ifndef CONNECTOR_MODULE_HPP
#define CONNECTOR_MODULE_HPP



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

void HandleExit(int sig);

const char* GetLANIPAddr();

void PrintServerInfo(uint16_t port);

class BaseConnectionInstance{
    protected:
        int sockfd;
    public:
        BaseConnectionInstance();
        bool CreateSocketFd();
        int GetFd();
        void CloseManually();
        virtual ~BaseConnectionInstance() noexcept;

};

class ServerInstance : public BaseConnectionInstance{
    private:
        uint16_t serv_port = 8000;
    public:
        int GetPort();
        void StartListening();
        void BindSocketToServer();
        int AcceptClient();
};

class ClientInstance : public BaseConnectionInstance{
    private:
        uint16_t serv_port;
    public:
        void ConnectToServer(const char* ip, uint16_t port);
        int GetPort();
};

void TerminateConnection(BaseConnectionInstance& ConnectionInstance);

#endif  /*CONNECTOR_MODULE_HPP*/