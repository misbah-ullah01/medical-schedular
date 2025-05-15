// Implements protocol message formatting and socket communication (Windows only)
#include "Protocol.h"
#include <cstring>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Protocol command constants
const string Protocol::SIGN_UP = "SIGN_UP";
const string Protocol::SIGN_IN = "SIGN_IN";
const string Protocol::REQUEST_APPOINTMENT = "REQUEST_APPOINTMENT";
const string Protocol::APPOINTMENT_APPROVAL = "APPOINTMENT_APPROVAL";
const string Protocol::APPOINTMENT_REJECTION = "APPOINTMENT_REJECTION";
const string Protocol::LIST_PENDING_APPOINTMENTS = "LIST_PENDING_APPOINTMENTS";
const string Protocol::ADMIN_APPROVE_APPOINTMENT = "ADMIN_APPROVE_APPOINTMENT";
const string Protocol::ADMIN_REJECT_APPOINTMENT = "ADMIN_REJECT_APPOINTMENT";
const string Protocol::GET_APPROVED_APPOINTMENTS = "GET_APPROVED_APPOINTMENTS";
const string Protocol::ERROR_MESSAGE = "ERROR";
const string Protocol::SUCCESS_MESSAGE = "SUCCESS";

// Send a message with length prefix
void Protocol::sendMessage(int clientSocket, const string &message)
{
    uint32_t messageLength = static_cast<uint32_t>(message.length());
    uint32_t netMessageLength = htonl(messageLength); // Convert to network byte order

    // Send the 4-byte length prefix
    if (::send(clientSocket, reinterpret_cast<const char *>(&netMessageLength), sizeof(uint32_t), 0) == SOCKET_ERROR)
    {
        cerr << "Server: Failed to send message length. Error: " << WSAGetLastError() << endl;
        return;
    }

    // Send the actual message body
    if (::send(clientSocket, message.c_str(), messageLength, 0) == SOCKET_ERROR)
    {
        cerr << "Server: Failed to send message body. Error: " << WSAGetLastError() << endl;
        return;
    }
}

// Receive a message with length prefix
string Protocol::receiveMessage(int socket)
{
    // First receive message length
    int messageLength = 0;
    if (recv(socket, reinterpret_cast<char *>(&messageLength), sizeof(int), 0) <= 0)
    {
        cerr << "Failed to receive message length" << endl;
        return "";
    }

    // Now receive the actual message
    vector<char> buffer(messageLength + 1);
    int bytesReceived = recv(socket, buffer.data(), messageLength, 0);
    if (bytesReceived <= 0)
    {
        cerr << "Failed to receive message" << endl;
        return "";
    }

    buffer[bytesReceived] = '\0';
    return string(buffer.data());
}

// Format a user message
string Protocol::createUserMessage(const string &username, const string &password)
{
    stringstream ss;
    ss << username << "|" << password;
    return ss.str();
}

// Format an appointment message
string Protocol::createAppointmentMessage(const string &appointmentDetails)
{
    return "APPOINTMENT|" + appointmentDetails;
}