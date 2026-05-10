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

    PL_CTL:
        000 -> Tells peer to listen for more. (Client/Server in speaking mode)
        100 -> Tells peer to speak. (Client/Server in listening mode)
*/


struct Packet{
    uint8_t PL_TYPE;                    /*1 BYTE*/
    uint32_t PL_LEN;                    /*4 BYTE*/
    const std::string PL_BODY;          /*dynamic*/
    uint8_t PL_CTL;                     /*1 BYTE*/
};

/*Never forget to tell this function which endiannes to use or the world is over.*/
template<typename idk>
void WritePacketBuffer(std::vector<uint8_t>& buff, const idk& value); 

std::vector<uint8_t> Serialize(Packet &data);

#pragma pack(pop)
#endif  /*COMMINCATION_MODULE*/