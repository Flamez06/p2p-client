#include "helper.h"
using namespace std;
bool sendAll(SOCKET sock, const char* data, int length){
    int totalSent = 0;
    while (totalSent < length){
        int bytesSent = send(sock,data+totalSent,length-totalSent,0);
        if (bytesSent == SOCKET_ERROR || bytesSent == 0){
            return false;
        }
        totalSent += bytesSent;
    }
    return true;
}

bool sendMessage(SOCKET sock, const string& message){
    uint32_t length = message.size();
    uint32_t netLength = htonl(length); // Convert to network byte order BIG ENDIAN

    if (!sendAll(sock,(char*)&netLength,sizeof(netLength))){
        return false;
    }
    if (!sendAll(sock,message.data(),length)){
        return false;
    }
    return true;
}

bool recvAll(SOCKET sock, char* buffer, int length){
    int totalReceived = 0;
    while (totalReceived < length){
        int bytesReceived = recv(sock,buffer+totalReceived,length-totalReceived,0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0){
            return false;
        }
        totalReceived += bytesReceived;
    }
    return true;
}

bool recvMessage(SOCKET sock, string& message){
    uint32_t netLength;
    if (!recvAll(sock,(char*)&netLength,sizeof(netLength))){
        return false;
    }
    uint32_t length = ntohl(netLength); // Convert from network byte order to host byte order

    message.resize(length);
    if (!recvAll(sock,message.data(),length)){
        return false;
    }
    return true;
}

