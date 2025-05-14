#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
using namespace std;

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

struct Message
{
    MessageType type;
    string payload;
};

class Protocol
{
public:
    static const string SIGN_UP;
    static const string SIGN_IN;
    static const string REQUEST_APPOINTMENT;
    static const string APPOINTMENT_APPROVAL;      // Added
    static const string APPOINTMENT_REJECTION;     // Added
    static const string LIST_PENDING_APPOINTMENTS; // New
    static const string ADMIN_APPROVE_APPOINTMENT; // New
    static const string ADMIN_REJECT_APPOINTMENT;  // New
    static const string GET_APPROVED_APPOINTMENTS; // For viewer
    static const string ERROR_MESSAGE;
    static const string SUCCESS_MESSAGE;

    void sendMessage(int socket, const string &message);
    string receiveMessage(int socket);
    string createUserMessage(const string &username, const string &password);
    string createAppointmentMessage(const string &appointmentDetails);
};

#endif // PROTOCOL_H