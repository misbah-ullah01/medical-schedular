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
#include "Admin.h"

using namespace std;

// Main server class for handling client connections and appointment management
class Server
{
public:
    Server();
    Server(int port);
    ~Server();

    // Start listening for clients
    void start();
    // Handle a single client connection
    void processClient(int clientSocket);
    // Handle sign-in request
    void handleSignIn(int clientSocket, const string &username, const string &password);
    // Handle sign-up request
    void handleSignUp(int clientSocket, const User &newUser);
    // Handle appointment request
    void handleAppointmentRequest(int clientSocket, const Appointment &appointment);
    // Approve appointment by ID
    void approveAppointment(int clientSocket, const string &appointmentId);
    // Reject appointment by ID
    void rejectAppointment(int clientSocket, const string &appointmentId);
    // List pending appointments for admin
    void handleListPendingAppointments(int clientSocket);
    // Approve appointment via admin command
    void handleAdminApproveAppointment(int clientSocket, const string &appointmentId);
    // Reject appointment via admin command
    void handleAdminRejectAppointment(int clientSocket, const string &appointmentId);
    // Send all appointments to viewer
    void handleGetApprovedAppointments(int clientSocket);
    // Get all appointments (for admin CLI)
    const vector<Appointment> &getAppointments() const { return appointments; }

private:
    // Send a response to a client
    void sendResponse(int clientSocket, const string &response);
    // Load users from file
    void loadUsers();
    // Save a new user to file
    void saveUser(const User &newUser);

    int port;
    int serverSocket;
    struct sockaddr_in serverAddr;
    map<string, User> users;
    vector<Appointment> appointments;
};

#endif // SERVER_H
