#pragma once

#ifndef CONNECTOR_MODULE_HPP
#define CONNECTOR_MODULE_HPP

#include <cstdint>

extern bool EnableDebug;
extern volatile bool ProgramRunning;

void HandleExit(int sig);

const char* GetLANIPAddr();

void PrintServerInfo(uint16_t port);

void TerminateConnection(BaseConnectionInstance& ConnectionInstance);

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
        ~ServerInstance() noexcept override;
};

class ClientInstance : public BaseConnectionInstance{
    private:
        uint16_t serv_port;
    public:
        void ConnectToServer(const char* ip, uint16_t port);
        int GetPort();
};

#endif  /*CONNECTOR_MODULE_HPP*/