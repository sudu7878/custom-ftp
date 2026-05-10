/*FileName: CommMod.hpp*/

#pragma once
#pragma pack(push, 1)

#include <cstdint>
#ifndef COMMUNICATION_MODULE
#define COMMUNICATION_MODULE

#include <stdbool.h>
#include <stdint.h>

#include <sys/errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <vector>


#include <ifaddrs.h>

/*Packet struct format:
    PL_TYPE:
        000 -> Messaging
        100 -> File     --TODO: Add support this later
        101 -> MessageBroadcast (note: only for server)

    PL_CTL:
        000 -> Nothing
        111 -> Tells peer to listen for more. (Client/Server in speaking mode)
        100 -> Tells peer to speak. (Client/Server in listening mode)
        010 -> Tells peer to end connection.
*/

/*this is the packet to send*/
struct Packet{
    uint8_t PL_TYPE;                    /*1 BYTE*/
    uint32_t PL_LEN;                    /*4 BYTE*/
    std::string PL_BODY;                /*dynamic*/
    uint8_t PL_CTL;                     /*1 BYTE*/
};

/*the following the packet to recieve*/
struct TemporaryPacketHeader{
    uint8_t type;
    uint32_t len;
};
struct TemporaryPacketBody{
    std::string body;
    uint8_t ctl;
};


enum PacketType{
    MESSAGE = 000,
    FILE_TRANSFER = 100,
    MESSAGE_BROADCAST = 101
};

enum ControlType{
    NO_ARG = 000,
    END_CONNECTION = 010
};

/*Never forget to tell this function which endiannes to use or the world is over.*/
template<typename idk>
void WritePacketBuffer(std::vector<uint8_t>& buff, const idk& value); 

std::vector<uint8_t> SerializePacket(Packet &data);   

TemporaryPacketHeader DeserializeHeaderPacket(const std::vector<uint8_t> &hdrbuff);
TemporaryPacketBody DeserializeBodyPacket(const std::vector<uint8_t> &buff, TemporaryPacketHeader &hdr);
Packet CombinePacket(TemporaryPacketHeader &hdr, TemporaryPacketBody &body);

#pragma pack(pop)
#endif  /*COMMINCATION_MODULE*/