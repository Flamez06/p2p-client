#include<iostream>
#include<winsock2.h>
#include<string>
using namespace std;

int main(){
    // Initialize winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed" << endl;
        return 1;
    }

    // Create a socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed" << endl;
        WSACleanup();
        return 1;
    }   

    // Set up the sockaddr_in structure for the server
    sockaddr_in srv{};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(8080); 
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");


    // Connect to the server
    int result = connect(clientSocket, (struct sockaddr*)&srv, sizeof(srv));

    if (result == SOCKET_ERROR) {
        cout << "Connection failed" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    } else {
        cout << "Connected to server" << endl;
    }


    // Send a message to the server
    string message;
    cout << "Enter message to send to server: ";
    getline(cin, message);
    send(clientSocket, message.c_str(), message.size(), 0);

    char buffer[1024] = {0};
    result = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (result > 0) {
        cout << "Received from server: " << buffer << endl;
    } else {
        cout << "Receive failed" << endl;
    }

    closesocket(clientSocket); 
    WSACleanup();
    return 0;

}