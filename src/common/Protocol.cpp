#include "Protocol.h"
#include <cstring>
#include <sstream>
#include <sys/types.h>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif
#include <iostream>
#include <vector>

using namespace std;

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

// Remove the problematic Protocol::send method entirely
// void Protocol::send(int socket, const char *message, int length, int flags)
// {
//    // First send message length
//    if (::send(socket, reinterpret_cast<const char *>(&length), sizeof(int), flags) == -1)
//    {
//        cerr << "Failed to send message length" << endl;
//        return;
//    }
//
//    // Then send the message
//    if (::send(socket, message, length, flags) == -1)
//    {
//        cerr << "Failed to send message" << endl;
//        return;
//    }
// }

void Protocol::sendMessage(int clientSocket, const string &message)
{
    uint32_t messageLength = static_cast<uint32_t>(message.length());
    uint32_t netMessageLength = htonl(messageLength); // Convert to network byte order

    // Send the 4-byte length prefix
    if (::send(clientSocket, reinterpret_cast<const char *>(&netMessageLength), sizeof(uint32_t), 0) == SOCKET_ERROR)
    {
#ifdef _WIN32
        cerr << "Server: Failed to send message length. Error: " << WSAGetLastError() << endl;
#else
        cerr << "Server: Failed to send message length. Error: " << strerror(errno) << endl;
#endif
        return;
    }

    // Send the actual message body
    if (::send(clientSocket, message.c_str(), messageLength, 0) == SOCKET_ERROR)
    {
#ifdef _WIN32
        cerr << "Server: Failed to send message body. Error: " << WSAGetLastError() << endl;
#else
        cerr << "Server: Failed to send message body. Error: " << strerror(errno) << endl;
#endif
        return;
    }
    // Optional: Log what was sent, but be mindful of log spam for large messages
    // cout << "[DEBUG Server] Sent length: " << messageLength << ", Sent message: " << message.substr(0, 100) << endl;
}

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

string Protocol::createUserMessage(const string &username, const string &password)
{
    stringstream ss;
    ss << username << "|" << password;
    return ss.str();
}

string Protocol::createAppointmentMessage(const string &appointmentDetails)
{
    return "APPOINTMENT|" + appointmentDetails;
}