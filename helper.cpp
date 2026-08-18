#include "helper.h"
#include<vector>
using namespace std;

void parseMessage(const string& message, string& type, string& portStr) {
    type.clear();
    portStr.clear();
    string word = "";
    for (char c : message) {    
        if (c != ' ') {
            word += c;
        } else {
            if (type.empty()) {
                type = word;
            } else if (portStr.empty()) {
                portStr = word;
            }
            word = "";
        }
    }
    if (!word.empty()) {
        if (type.empty()) {
            type = word;
        } else if (portStr.empty()) {
            portStr = word;
        }
    }
}


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

// MESSAGE PROTOCOL
// 4 BYTES | MESSAGE TYPE
// 4 BYTES | PIECE INDEX
// 4 BYTES | PIECE LENGTH 
// N BYTES | MESSAGE DATA
bool sendPiece(SOCKET sock,uint32_t pieceIndex,const vector<char>& piece){
    uint32_t netType = htonl(MSG_PIECE);
    if(!sendAll(sock,(char*)&netType,sizeof(netType))){
        return false;
    }
    uint32_t netPI = htonl(pieceIndex);
    if(!sendAll(sock,(char*)&netPI,sizeof(netPI))){
        return false;
    }
    uint32_t length = piece.size();
    uint32_t netLength = htonl(length);
    if(!sendAll(sock,(char*)&netLength,sizeof(netLength))){
        return false;
    }
    if(!sendAll(sock,piece.data(),length)){
        return false;
    }
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

