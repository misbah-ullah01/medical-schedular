// Implements TCP client for communicating with the server
#include "NetworkClient.h"
#include "Protocol.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <cstdint>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

using namespace std;

const string DEFAULT_SERVER_IP = "127.0.0.1";
const int DEFAULT_SERVER_PORT = 12345;

// Helper to get last socket error as string
string get_socket_error_client()
{
#ifdef _WIN32
    return to_string(WSAGetLastError());
#else
    return strerror(errno);
#endif
}

// Initialize with server address and port
NetworkClient::NetworkClient(const string &serverAddress, int serverPort)
    : serverAddress(serverAddress), serverPort(serverPort), socketFD(INVALID_SOCKET)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "WSAStartup failed with error: " << WSAGetLastError() << endl;
        return;
    }
}

// Default constructor uses localhost and default port
NetworkClient::NetworkClient()
    : serverAddress(DEFAULT_SERVER_IP), serverPort(DEFAULT_SERVER_PORT), socketFD(INVALID_SOCKET)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "WSAStartup failed with error: " << WSAGetLastError() << endl;
        return;
    }
}

// Cleanup
NetworkClient::~NetworkClient()
{
    disconnect();
    WSACleanup();
}

// Connect to the server
bool NetworkClient::connectToServer()
{
    cout << "Attempting to connect to server at " << serverAddress << ":" << serverPort << endl;
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == INVALID_SOCKET)
    {
        cerr << "Socket creation error: " << WSAGetLastError() << endl;
        return false;
    }
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    if (inet_pton(AF_INET, serverAddress.c_str(), &serverAddr.sin_addr) <= 0)
    {
        cerr << "Invalid address/ Address not supported: " << WSAGetLastError() << endl;
        closesocket(socketFD);
        socketFD = INVALID_SOCKET;
        return false;
    }
    if (connect(socketFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cerr << "Connection to server failed with error: " << WSAGetLastError() << endl;
        closesocket(socketFD);
        socketFD = INVALID_SOCKET;
        return false;
    }
    cout << "Successfully connected to server!" << endl;
    return true;
}

// Disconnect from server
void NetworkClient::disconnect()
{
    if (socketFD != INVALID_SOCKET)
    {
        closesocket(socketFD);
        socketFD = INVALID_SOCKET;
    }
}

// Send a generic request to the server
bool NetworkClient::sendRequest(const string &request)
{
    int host_messageLength = static_cast<int>(request.length());
    const int MAX_MESSAGE_LENGTH = 4096;
    if (host_messageLength <= 0 || host_messageLength > MAX_MESSAGE_LENGTH)
    {
        cerr << "[ERROR Client] Message length (" << host_messageLength
             << ") is invalid or exceeds the maximum allowed length of "
             << MAX_MESSAGE_LENGTH << " bytes. Request not sent." << endl;
        return false;
    }
    uint32_t net_messageLength = htonl(static_cast<uint32_t>(host_messageLength));
    if (send(socketFD, reinterpret_cast<const char *>(&net_messageLength), sizeof(uint32_t), 0) == SOCKET_ERROR)
    {
        cerr << "Failed to send message length. Error: " << get_socket_error_client() << endl;
        return false;
    }
    if (send(socketFD, request.c_str(), host_messageLength, 0) == SOCKET_ERROR)
    {
        cerr << "Failed to send message body. Error: " << get_socket_error_client() << endl;
        return false;
    }
    return true;
}

// Receive a response from the server
string NetworkClient::receiveResponse()
{
    uint32_t net_messageLength = 0;
    int lenRecv = recv(socketFD, reinterpret_cast<char *>(&net_messageLength), sizeof(uint32_t), 0);
    if (lenRecv <= 0)
    {
        if (lenRecv == 0)
        {
            cerr << "Server closed connection while waiting for response length." << endl;
        }
        else
        {
            cerr << "Failed to receive response length. Error: " << get_socket_error_client() << endl;
        }
        return "";
    }
    if (lenRecv != sizeof(uint32_t))
    {
        cerr << "Failed to receive the full response length. Expected " << sizeof(uint32_t) << " bytes, got " << lenRecv << " bytes." << endl;
        return "";
    }
    uint32_t host_messageLength = ntohl(net_messageLength);
    if (host_messageLength == 0 || host_messageLength > 4096)
    {
        cerr << "Invalid response message length received: " << host_messageLength << ". Must be > 0 and <= 4096." << endl;
        return "";
    }
    vector<char> buffer(host_messageLength);
    int messageBodyBytesToReceive = static_cast<int>(host_messageLength);
    int bytesReceived = recv(socketFD, buffer.data(), messageBodyBytesToReceive, 0);
    if (bytesReceived <= 0)
    {
        if (bytesReceived == 0)
        {
            cerr << "Server closed connection after sending length, while waiting for response body." << endl;
        }
        else
        {
            cerr << "Failed to receive response body. Error: " << get_socket_error_client() << endl;
        }
        return "";
    }
    if (bytesReceived != messageBodyBytesToReceive)
    {
        cerr << "Failed to receive the full response body. Expected " << messageBodyBytesToReceive
             << " bytes, got " << bytesReceived << " bytes. Partial message: \""
             << string(buffer.data(), bytesReceived) << "\"" << endl;
        return "";
    }
    return string(buffer.data(), bytesReceived);
}

// Send sign-up request to server
bool NetworkClient::sendSignUpRequest(const string &username, const string &encryptedPassword)
{
    if (username.empty() || encryptedPassword.empty())
    {
        cerr << "Username or password cannot be empty" << endl;
        return false;
    }
    stringstream ss;
    ss << Protocol::SIGN_UP << "|" << username << "|" << encryptedPassword;
    if (!sendRequest(ss.str()))
    {
        cerr << "Failed to send sign-up request" << endl;
        return false;
    }
    string response = receiveResponse();
    return response.find(Protocol::SUCCESS_MESSAGE) != string::npos;
}

// Send sign-in request to server
bool NetworkClient::sendSignInRequest(const string &username, const string &encryptedPassword)
{
    if (username.empty() || encryptedPassword.empty())
    {
        cerr << "Username or password cannot be empty" << endl;
        return false;
    }
    stringstream ss;
    ss << Protocol::SIGN_IN << "|" << username << "|" << encryptedPassword;
    if (!sendRequest(ss.str()))
    {
        cerr << "Failed to send sign-in request" << endl;
        return false;
    }
    string response = receiveResponse();
    return response.find(Protocol::SUCCESS_MESSAGE) != string::npos;
}

// Send appointment request to server
bool NetworkClient::sendAppointmentRequest(const string &appointmentDetails)
{
    if (appointmentDetails.empty())
    {
        cerr << "Appointment details cannot be empty" << endl;
        return false;
    }
    stringstream ss;
    ss << Protocol::REQUEST_APPOINTMENT << "|" << appointmentDetails;
    if (!sendRequest(ss.str()))
    {
        cerr << "Failed to send appointment request" << endl;
        return false;
    }
    string response = receiveResponse();
    return response.find(Protocol::SUCCESS_MESSAGE) != string::npos;
}