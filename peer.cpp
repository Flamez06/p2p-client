#include "fileManager.h"
#include "helper.h"
#include <cstdint>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <winsock2.h>

using namespace std;

// Mutex for locking stuff
mutex peerMutex;

struct peerInfo {
    string ip;
    int port;
};

// Store known peers in a vector
vector<peerInfo> knownPeers;

bool addPeer(const string &ip, int port) {
    lock_guard<mutex> lock(peerMutex);
    for (const auto &peer : knownPeers) {
        if (peer.ip == ip && peer.port == port) {
            return false; // Peer already exists
        }
    }
    peerInfo newPeer{ip, port};
    knownPeers.push_back(newPeer);
    return true;
}

void handlePeer(SOCKET peerSocket, string peerIP) {
    string receivedMessage;
    if (!recvMessage(peerSocket, receivedMessage)) {
        cout << "Receive failed." << endl;
    } else {
        string type, portStr;
        cout << "Received from peer: " << receivedMessage << endl;
        parseMessage(receivedMessage, type, portStr);

        if (type == "HANDSHAKE") {
            string message = "HANDSHAKE_OK";
            addPeer(peerIP, stoi(portStr));
            sendMessage(peerSocket, message);
            message = "Connected to peer on port " + portStr;
            cout << message << endl;
        }
    }

    closesocket(peerSocket);
}

void listenForPeers(int myPort) {
    while (true) {
        SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockfd == INVALID_SOCKET) {
            cout << "Socket creation failed." << endl;
            return;
        }

        sockaddr_in srv{};
        srv.sin_family = AF_INET;
        srv.sin_port = htons(myPort);
        srv.sin_addr.s_addr = INADDR_ANY;

        if (bind(sockfd, (sockaddr *)&srv, sizeof(srv)) == SOCKET_ERROR) {
            cout << "Socket binding failed." << endl;
            closesocket(sockfd);
            return;
        }
        // Start listening
        if (listen(sockfd, 5) == SOCKET_ERROR) {
            cout << "Listening failed." << endl;
            closesocket(sockfd);
            return;
        }
        cout << "Listening on port " << myPort << endl;

        sockaddr_in connectedAddr{};
        int connectedAddrLen = sizeof(connectedAddr);
        SOCKET connectedSocket = accept(sockfd, (sockaddr *)&connectedAddr, &connectedAddrLen);

        if (connectedSocket == INVALID_SOCKET) {
            cout << "Accepting connection failed." << endl;
            closesocket(sockfd);
            return;
        } else {
            cout << "Peer connected from " << inet_ntoa(connectedAddr.sin_addr) << ":" << ntohs(connectedAddr.sin_port)
                 << endl;
            thread peerThread(handlePeer, connectedSocket, inet_ntoa(connectedAddr.sin_addr));
            peerThread.detach();
        }
        closesocket(sockfd);
    }
}

void connectToPeer(string peerIP, int peerPort, int myPort) {
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd == INVALID_SOCKET) {
        cout << "Socket creation failed." << endl;
        return;
    }

    sockaddr_in peerAddr{};
    peerAddr.sin_family = AF_INET;
    peerAddr.sin_port = htons(peerPort);
    peerAddr.sin_addr.s_addr = inet_addr(peerIP.c_str());

    if (connect(sockfd, (sockaddr *)&peerAddr, sizeof(peerAddr)) == SOCKET_ERROR) {
        cout << "Connection to peer failed." << endl;
        closesocket(sockfd);
        return;
    }

    cout << "Connected to peer " << peerIP << ":" << peerPort << endl;

    string handshake = "HANDSHAKE " + to_string(myPort);

    if (!sendMessage(sockfd, handshake)) {
        cout << "Failed to send handshake." << endl;
        closesocket(sockfd);
        return;
    }

    string response;

    if (!recvMessage(sockfd, response)) {
        cout << "Failed to receive handshake response." << endl;
        closesocket(sockfd);
        return;
    }

    cout << "Received from peer: " << response << endl;

    closesocket(sockfd);
}

int main(int argc, char *argv[]) {
    fileMetadata mdata;
    createFileMetadata("test.txt",mdata);
    for(uint32_t i = 0; i<mdata.pieceCount; i++){
        cout << "Piece hash " << i << " : " << mdata.pieceHashes[i] << endl;
    }
    
    if (argc != 2 && argc != 4) {
        cout << "Usage:" << endl;
        cout << "  peer.exe <port>" << endl;
        cout << "  peer.exe <port> <peer_ip> <peer_port>" << endl;
        return 1;
    }

    int port = stoi(argv[1]);

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed." << endl;
        return 1;
    }

    // Incoming connections happen here
    thread listenerThread(listenForPeers, port);

    // Outgoing connection happens here
    if (argc == 4) {
        string peerIP = argv[2];
        int peerPort = stoi(argv[3]);

        connectToPeer(peerIP, peerPort, port);
    }

    listenerThread.join();

    WSACleanup();

    return 0;
}