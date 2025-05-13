#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <string>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include "Protocol.h"

using namespace std;

class NetworkClient
{
public:
    NetworkClient();
    NetworkClient(const string &serverAddress, int serverPort);
    ~NetworkClient();

    bool connectToServer();
    void disconnect();
    bool sendRequest(const string &request);
    bool sendSignUpRequest(const string &username, const string &encryptedPassword);
    bool sendSignInRequest(const string &username, const string &encrpytedPassword);
    bool sendAppointmentRequest(const string &appointmentDetails);
    string receiveResponse();

private:
    string serverAddress;
    int serverPort;
    SOCKET socketFD;
    struct sockaddr_in serverAddr;
};

#endif // NETWORKCLIENT_H