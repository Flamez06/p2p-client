#include <winsock2.h>
#include <iostream>
#include <string>
#define port 8080
using namespace std;


int main() {
    SOCKET sockfd;
    int nret=0;
    WSADATA wsaData;
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed" << endl;
        return 1;
    }

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sockfd != INVALID_SOCKET) {
        cout << "Socket created successfully" << endl;
    } else {
        cout << "Socket creation failed" << endl;
        WSACleanup();
        return 1;
    }

    // Set up the sockaddr_in structure
    sockaddr_in srv{};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket
    nret=bind(sockfd, (struct sockaddr*)&srv, sizeof(srv));
    if (nret == 0) {
        cout << "Socket bound successfully" << endl;
    } else {
        cout << "Socket binding failed" << endl;
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    nret=listen(sockfd, 5);
    if (nret < 0) {
        cout << "Listening failed" << endl;
        closesocket(sockfd);
        WSACleanup();
        return 1;
    } else {
        cout << "Listening on port " << port << endl;
    }

    // Accept a client connection
    SOCKET connectedSocket=accept(sockfd, NULL, NULL);
    char buffer[1024] = {0};
    nret=recv(connectedSocket, buffer, sizeof(buffer), 0);
    if (nret > 0) {
        cout << "Received from client: " << buffer << endl;
    } else {
        cout << "Receive failed" << endl;
    }

    string response;
    cout << "Enter response to send to client: ";
    getline(cin, response);
    send(connectedSocket, response.c_str(), strlen(response.c_str()), 0);

    //clean up
    closesocket(connectedSocket);
    closesocket(sockfd);
    WSACleanup();

    return 0;
}