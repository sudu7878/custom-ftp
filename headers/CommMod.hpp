#pragma once

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
#include <iostream>
#include <string>

#ifndef COMMUNICATION_MODULE
#define COMMUNICATION_MODULE

struct Packet{
    uint8_t PL_TYPE;
    uint16_t PL_LEN;
    const char * PL_TXT;
};

int SizeOfMessagePacket(Packet& packet);

bool SendPacket(int sockfd, Packet& packet, size_t size);

#endif  /*COMMINCATION_MODULE*/