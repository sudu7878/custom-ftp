#pragma once

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