#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include "User.h"
#include "Protocol.h"
#include "Appointment.h"
#include "Admin.h" // Make sure Admin.h is included

using namespace std;

class Server
{
public:
    Server();
    Server(int port);
    ~Server();

    void start();
    void processClient(int clientSocket);
    void handleSignIn(int clientSocket, const string &username, const string &password);
    void handleSignUp(int clientSocket, const User &newUser);
    void handleAppointmentRequest(int clientSocket, const Appointment &appointment);
    void approveAppointment(int clientSocket, const string &appointmentId);
    void rejectAppointment(int clientSocket, const string &appointmentId);

    // New handlers for client-driven admin actions and viewer
    void handleListPendingAppointments(int clientSocket);
    void handleAdminApproveAppointment(int clientSocket, const string &appointmentId);
    void handleAdminRejectAppointment(int clientSocket, const string &appointmentId);
    void handleGetApprovedAppointments(int clientSocket);

    // Expose appointments for admin CLI
    const vector<Appointment> &getAppointments() const { return appointments; }

private:
    void sendResponse(int clientSocket, const string &response); // Added declaration

    // Persistence helpers for users
    void loadUsers();
    void saveUser(const User &newUser);

    int port;
    int serverSocket;
    struct sockaddr_in serverAddr;

    map<string, User> users;          // Registered users
    vector<Appointment> appointments; // Stored appointments
};

#endif // SERVER_H
