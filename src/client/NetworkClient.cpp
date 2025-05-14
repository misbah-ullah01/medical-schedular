#include "NetworkClient.h"
#include "Protocol.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <vector>
#include <cstdint> // For uint32_t
#ifdef _WIN32
#include <winsock2.h> // For htonl. Must be included before windows.h if that is used.
#else
#include <arpa/inet.h> // For htonl
#endif

using namespace std;

// Set default server IP and port as string and int
const string DEFAULT_SERVER_IP = "127.0.0.1"; // Default to localhost
const int DEFAULT_SERVER_PORT = 12345;        // Make sure this matches your server

// Helper to get error message
// Placed here to be available for NetworkClient methods
string get_socket_error_client() // Renamed to avoid potential ODR violations if linked with server in some contexts, though unlikely for separate executables.
{
#ifdef _WIN32
    return to_string(WSAGetLastError());
#else
    return strerror(errno); // errno is from <cerrno> or <errno.h>
#endif
}

NetworkClient::NetworkClient(const string &serverAddress, int serverPort)
    : serverAddress(serverAddress), serverPort(serverPort), socketFD(INVALID_SOCKET)
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "WSAStartup failed with error: " << WSAGetLastError() << endl;
        return;
    }
}

NetworkClient::NetworkClient()
    : serverAddress(DEFAULT_SERVER_IP), serverPort(DEFAULT_SERVER_PORT), socketFD(INVALID_SOCKET)
{
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cerr << "WSAStartup failed with error: " << WSAGetLastError() << endl;
        return;
    }
}

NetworkClient::~NetworkClient()
{
    disconnect();
    WSACleanup();
}

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

    // Use c_str() for inet_pton
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

void NetworkClient::disconnect()
{
    if (socketFD != INVALID_SOCKET)
    {
        closesocket(socketFD);
        socketFD = INVALID_SOCKET;
    }
}

bool NetworkClient::sendRequest(const string &request)
{
    int host_messageLength = static_cast<int>(request.length());
    // Log the length that will be sent and a preview of the request
    // cout << "[DEBUG Client] Attempting to send request. Calculated host_messageLength: " << host_messageLength << endl;
    // if (!request.empty())
    // { // Add a small preview if not empty
    //     cout << "[DEBUG Client] Request content preview (first 100 chars): " << request.substr(0, 100) << endl;
    // }
    // else
    // {
    //     cout << "[DEBUG Client] Request content is empty." << endl;
    // }

    // Client-side check for message length (mirroring server's limit)
    // The server appears to have a limit of 4096 bytes for the message body.
    const int MAX_MESSAGE_LENGTH = 4096;
    if (host_messageLength <= 0 || host_messageLength > MAX_MESSAGE_LENGTH)
    {
        cerr << "[ERROR Client] Message length (" << host_messageLength
             << ") is invalid or exceeds the maximum allowed length of "
             << MAX_MESSAGE_LENGTH << " bytes. Request not sent." << endl;
        return false; // Prevent sending invalid/oversized message
    }

    // Convert message length to network byte order
    uint32_t net_messageLength = htonl(static_cast<uint32_t>(host_messageLength));

    // cout << "[DEBUG Client] Sending message length (host: " << host_messageLength << ", net_uint32: " << net_messageLength << ")" << endl;
    if (send(socketFD, reinterpret_cast<const char *>(&net_messageLength), sizeof(uint32_t), 0) == SOCKET_ERROR)
    {
        cerr << "Failed to send message length. Error: " << get_socket_error_client() << endl;
        return false;
    }
    // cout << "[DEBUG Client] Message length sent successfully." << endl;

    // Send the actual message
    // cout << "[DEBUG Client] Sending message body (" << host_messageLength << " bytes)..." << endl;
    if (send(socketFD, request.c_str(), host_messageLength, 0) == SOCKET_ERROR)
    {
        cerr << "Failed to send message body. Error: " << get_socket_error_client() << endl; // This is where 10053 was reported
        return false;
    }
    // cout << "[DEBUG Client] Message body sent successfully." << endl;

    return true;
}

string NetworkClient::receiveResponse()
{
    uint32_t net_messageLength = 0;
    // Receive the 4-byte message length in network byte order
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
        return ""; // Return empty string or throw an exception
    }
    if (lenRecv != sizeof(uint32_t))
    {
        cerr << "Failed to receive the full response length. Expected " << sizeof(uint32_t) << " bytes, got " << lenRecv << " bytes." << endl;
        return "";
    }

    uint32_t host_messageLength = ntohl(net_messageLength);
    // cout << "[DEBUG Client] Received net_messageLength: " << net_messageLength
    //           << ", converted to host_messageLength: " << host_messageLength << endl;

    if (host_messageLength == 0 || host_messageLength > 4096)
    { // Max length 4KB, adjust as needed
        cerr << "Invalid response message length received: " << host_messageLength << ". Must be > 0 and <= 4096." << endl;
        return "";
    }

    vector<char> buffer(host_messageLength);
    int messageBodyBytesToReceive = static_cast<int>(host_messageLength);
    int bytesReceived = recv(socketFD, buffer.data(), messageBodyBytesToReceive, 0); // MSG_WAITALL might be too blocking for client if server sends partial

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
        // Optionally, could try to process the partial message or loop to receive more,
        // but for now, treating as an error.
        return ""; // Or return the partial data if appropriate for the protocol
    }

    return string(buffer.data(), bytesReceived);
}

bool NetworkClient::sendSignUpRequest(const string &username, const string &encryptedPassword)
{
    if (username.empty() || encryptedPassword.empty())
    {
        cerr << "Username or password cannot be empty" << endl;
        return false;
    }

    // Format: "SIGN_UP|username|password"
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

bool NetworkClient::sendSignInRequest(const string &username, const string &encryptedPassword)
{
    if (username.empty() || encryptedPassword.empty())
    {
        cerr << "Username or password cannot be empty" << endl;
        return false;
    }

    // Format: "SIGN_IN|username|password"
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

bool NetworkClient::sendAppointmentRequest(const string &appointmentDetails)
{
    if (appointmentDetails.empty())
    {
        cerr << "Appointment details cannot be empty" << endl;
        return false;
    }

    // Format: "REQUEST_APPOINTMENT|details"
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