#include "helper.h"
#include <cstdint>
#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>

using namespace std;

void listenForPeers(int port) {
    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == INVALID_SOCKET) {
        cout << "Socket creation failed." << endl;
        return;
    }

    sockaddr_in srv{};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
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
    cout << "Listening on port " << port << endl;

    sockaddr_in connectedAddr{};
    int connectedAddrLen = sizeof(connectedAddr);
    SOCKET connectedSocket = accept(sockfd, (sockaddr *)&connectedAddr, &connectedAddrLen);

    if (connectedSocket == INVALID_SOCKET) {
        cout << "Accepting connection failed." << endl;
        closesocket(sockfd);
        return;
    }
    cout << "Peer connected from " << inet_ntoa(connectedAddr.sin_addr) << ":" << ntohs(connectedAddr.sin_port) << endl;

    string receivedMessage;
    if (!recvMessage(connectedSocket, receivedMessage)) {
        cout << "Receive failed." << endl;
    } else {
        cout << "Received from peer: " << receivedMessage << endl;
        string type;
        string portStr;
        string word = "";
        for (char c : receivedMessage) {
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

        if (type == "HANDSHAKE") {
            string message = "HANDSHAKE_OK";
            sendMessage(connectedSocket, message);
            message = "Connected to peer on port " + portStr;
            cout << message << endl;
        }
    }

    closesocket(connectedSocket);
    closesocket(sockfd);
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