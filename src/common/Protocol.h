#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
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

// Handles protocol constants and message formatting/parsing
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
    // Send a message to a socket
    void sendMessage(int socket, const string &message);
    // Receive a message from a socket
    string receiveMessage(int socket);
    // Format a user message
    string createUserMessage(const string &username, const string &password);
    // Format an appointment message
    string createAppointmentMessage(const string &appointmentDetails);
};

#endif // PROTOCOL_H