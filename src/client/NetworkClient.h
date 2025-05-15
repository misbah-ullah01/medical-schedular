#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <string>
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include "Protocol.h"

using namespace std;

// Handles TCP communication with the server for client operations (Windows only)
class NetworkClient
{
public:
    NetworkClient();                                            // Default constructor
    NetworkClient(const string &serverAddress, int serverPort); // Initialize with server address/port
    ~NetworkClient();                                           // Cleanup

    bool connectToServer();                                                          // Connect to server
    void disconnect();                                                               // Disconnect from server
    bool sendRequest(const string &request);                                         // Send a generic request
    bool sendSignUpRequest(const string &username, const string &encryptedPassword); // Send sign-up
    bool sendSignInRequest(const string &username, const string &encrpytedPassword); // Send sign-in
    bool sendAppointmentRequest(const string &appointmentDetails);                   // Send appointment request
    string receiveResponse();                                                        // Receive response from server

private:
    string serverAddress;
    int serverPort;
    SOCKET socketFD;
    struct sockaddr_in serverAddr;
};

#endif // NETWORKCLIENT_H