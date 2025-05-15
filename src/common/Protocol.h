#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

// Enum for different message types in the protocol
enum class MessageType
{
    MSG_SIGN_UP,
    MSG_SIGN_IN,
    MSG_APPOINTMENT_REQUEST,
    MSG_APPOINTMENT_APPROVAL,
    MSG_APPOINTMENT_REJECTION,
    MSG_ERROR,
    MSG_SUCCESS
};

// Represents a protocol message
struct Message
{
    MessageType type;
    string payload;
};

// Handles protocol constants and message formatting/parsing (Windows only)
class Protocol
{
public:
    static const string SIGN_UP;
    static const string SIGN_IN;
    static const string REQUEST_APPOINTMENT;
    static const string APPOINTMENT_APPROVAL;
    static const string APPOINTMENT_REJECTION;
    static const string LIST_PENDING_APPOINTMENTS;
    static const string ADMIN_APPROVE_APPOINTMENT;
    static const string ADMIN_REJECT_APPOINTMENT;
    static const string GET_APPROVED_APPOINTMENTS;
    static const string ERROR_MESSAGE;
    static const string SUCCESS_MESSAGE;
    void sendMessage(int socket, const string &message);
    string receiveMessage(int socket);
    string createUserMessage(const string &username, const string &password);
    string createAppointmentMessage(const string &appointmentDetails);
};

#endif // PROTOCOL_H