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

std::vector<uint8_t> SerializedPacket(Packet &data){
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