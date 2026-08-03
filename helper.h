#include <winsock2.h>
#include <string>
#include <cstdint>

bool sendAll(SOCKET sock, const char* data, int length);
bool recvAll(SOCKET sock, char* data, int length);
bool sendMessage(SOCKET sock, const std::string& message);
bool recvMessage(SOCKET sock, std::string& message);