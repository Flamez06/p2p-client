#pragma once 
#include <winsock2.h>
#include <string>
#include<vector>
#include <cstdint>
const uint32_t MSG_REQUEST_PIECE = 1;
const uint32_t MSG_PIECE = 2;
void parseMessage(const std::string& message, std::string& type, std::string& portStr);
bool sendAll(SOCKET sock, const char* data, int length);
bool recvAll(SOCKET sock, char* data, int length);
bool sendMessage(SOCKET sock, const std::string& message);
bool sendPiece(SOCKET sock,uint32_t pieceIndex,const std::vector<char>& piece);
bool recvMessage(SOCKET sock, std::string& message);