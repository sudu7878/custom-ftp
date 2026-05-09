/*FileName: UserHandler.hpp*/

#pragma onces
#ifndef SERVER_MODULE_HPP
#define SERVER_MODULE_HPP

#include <stdint.h>

int StartServer();
int StartClient(const char* ip, uint16_t port);

#endif /*USER_HANDLER_HPP*/