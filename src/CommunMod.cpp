/*FileName: CommMod.cpp*/

#include "CommunMod.hpp"
#include "ClientMod.hpp"
#include "ServerMod.hpp"

#include <cstddef>
#include <cstdint>
#include <stdbool.h>
#include <stdint.h>

#include <sys/errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <vector>
#include <cstring>
#include <arpa/inet.h>

#include <ifaddrs.h>

size_t PacketSize = 0;

template<typename idk>
void WritePacketBuffer(std::vector<uint8_t> &buff, const idk &value){
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value);
    for(size_t i = 0; i <sizeof(idk); i++){
        buff.push_back(bytes[i]);
    }
}

std::vector<uint8_t> SerializePacket(Packet &data){
    std::vector<uint8_t> PacketBuff;
    
    PacketBuff.push_back(data.PL_TYPE);

    data.PL_LEN = data.PL_BODY.size();  /*size of the str*/
    WritePacketBuffer(PacketBuff, htonl(data.PL_LEN));

    for(char StringChars : data.PL_BODY){
        PacketBuff.push_back((uint8_t)StringChars);
    }

    PacketBuff.push_back(data.PL_CTL);
    
    return PacketBuff;
}

TemporaryPacketHeader DeserializeHeaderPacket(const std::vector<uint8_t> &hdrbuff){
    TemporaryPacketHeader RecievedHeaderPacket;
    size_t index = 0;

    RecievedHeaderPacket.type = hdrbuff[index];
        index += sizeof(uint8_t);
        uint32_t Length;

    memcpy(&Length, &hdrbuff[index], sizeof(uint32_t));
    RecievedHeaderPacket.len = ntohl(Length);
    return RecievedHeaderPacket;
}

TemporaryPacketBody DeserializeBodyPacket(const std::vector<uint8_t> &buff, TemporaryPacketHeader &hdr){
    TemporaryPacketBody RecievedBodyPacket;

    size_t index = 0;
    
    for(size_t i = 0; i < hdr.len; i++){
        RecievedBodyPacket.body += (char)buff[index];
        index++;
    }

    RecievedBodyPacket.ctl = buff[index];

    return RecievedBodyPacket;
}

Packet CombinePacket(TemporaryPacketHeader &hdr, TemporaryPacketBody &body){
   Packet CombinedPacket;
   CombinedPacket.PL_TYPE = hdr.type;
   CombinedPacket.PL_LEN = hdr.len;
   CombinedPacket.PL_BODY = body.body;
   CombinedPacket.PL_CTL = body.ctl;
   return CombinedPacket;
}  