#include <iostream>
#include <string>
#include <winsock2.h>
#include <cstdint>

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


int main(int argc, char* argv[]){
    if (argc != 2 && argc != 4){
        cout << "Usage:" << endl;
        cout << "  peer.exe <port>" << endl;
        cout << "  peer.exe <port> <peer_ip> <peer_port>" << endl;
        return 1;
    }

    int port = stoi(argv[1]);

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0){
        cout << "WSAStartup failed." << endl;
        return 1;
    }


    // Create a socket
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET){
        cout << "Socket creation failed." << endl;
        WSACleanup();
        return 1;
    }

    // Listening peer

    if (argc == 2){
        sockaddr_in srv{};
        srv.sin_family = AF_INET;
        srv.sin_port = htons(port);
        srv.sin_addr.s_addr = INADDR_ANY;

        // Bind listening socket to our port
        if (bind(sockfd, (sockaddr*)&srv, sizeof(srv)) == SOCKET_ERROR){
            cout << "Socket binding failed." << endl;
            closesocket(sockfd);
            WSACleanup();
            return 1;
        }

        // Start listening
        if (listen(sockfd, 5) == SOCKET_ERROR){
            cout << "Listening failed." << endl;
            closesocket(sockfd);
            WSACleanup();
            return 1;
        }
        cout << "Listening on port " << port << endl;

        sockaddr_in connectedAddr{};
        int connectedAddrLen = sizeof(connectedAddr);
        SOCKET connectedSocket = accept(sockfd,(sockaddr*)&connectedAddr,&connectedAddrLen);

        if (connectedSocket == INVALID_SOCKET){
            cout << "Accepting connection failed." << endl;
            closesocket(sockfd);
            WSACleanup();
            return 1;
        }

        cout << "Peer connected from "<< inet_ntoa(connectedAddr.sin_addr)<< ":"<< ntohs(connectedAddr.sin_port)<< endl;
        
        string receivedMessage;
        if (!recvMessage(connectedSocket, receivedMessage)){
            cout << "Receive failed." << endl;
        }
        else{
            cout << "Received from peer: " << receivedMessage << endl;
        }
    
        cout << "Enter message to send to peer: ";
        string message;
        getline(cin, message);
        sendMessage(connectedSocket, message);
        closesocket(connectedSocket);
    }

    // Sending peer
    else{
        string peerIP = argv[2];
        int peerPort = stoi(argv[3]);

        // Address of the peer we want to connect to
        sockaddr_in peerAddr{};
        peerAddr.sin_family = AF_INET;
        peerAddr.sin_port = htons(peerPort);
        peerAddr.sin_addr.s_addr = inet_addr(peerIP.data());

        if (connect(sockfd,(sockaddr*)&peerAddr,sizeof(peerAddr)) == SOCKET_ERROR){
            cout << "Connection to peer failed." << endl;
            closesocket(sockfd);
            WSACleanup();
            return 1;
        }
        cout << "Connected to peer "<< peerIP<< ":"<< peerPort<< endl;

        cout << "Enter message to send to peer: ";
        string message;
        getline(cin, message);
        sendMessage(sockfd, message);

        string receivedMessage;
        if (!recvMessage(sockfd, receivedMessage)){
            cout << "Receive failed." << endl;
        }
        else{
            cout << "Received from peer: " << receivedMessage << endl;
        }
    }
    closesocket(sockfd);
    WSACleanup();
    return 0;
}