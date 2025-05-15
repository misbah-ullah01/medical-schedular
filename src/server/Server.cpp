// Implements Server class for handling clients and appointments (Windows only)
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <cstdint>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <mutex>
#include "Server.h"
#include "Protocol.h"
#include "User.h"
#include "Appointment.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Helper to get last socket error as string
string get_socket_error()
{
    return to_string(WSAGetLastError());
}

mutex g_appointments_mutex;
mutex g_users_mutex;

// Initialize server on default port
Server::Server() : Server(12345) {}

// Initialize server on given port
Server::Server(int port) : port(port), serverSocket(INVALID_SOCKET)
{
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0)
    {
        cerr << "WSAStartup failed: " << wsaResult << endl;
        exit(EXIT_FAILURE);
    }
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        cerr << "Socket creation failed: " << get_socket_error() << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        cerr << "Bind failed: " << get_socket_error() << endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    if (listen(serverSocket, 3) == SOCKET_ERROR)
    {
        cerr << "Listen failed: " << get_socket_error() << endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    cout << "Server initialized on port " << port << endl;
    loadUsers();
}

// Cleanup
Server::~Server()
{
    if (serverSocket != INVALID_SOCKET)
    {
        closesocket(serverSocket);
    }
    WSACleanup();
    cout << "Server shut down." << endl;
}

// Send a response to a client
void Server::sendResponse(int clientSocket, const string &response)
{
    Protocol protocol;
    protocol.sendMessage(clientSocket, response);
}

// Start listening for clients and handle each in a thread
void Server::start()
{
    cout << "[INFO] Server listening on port " << port << "..." << endl;
    vector<thread> client_threads;
    while (true)
    {
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            cerr << "[ERROR] Accept failed: " << get_socket_error() << endl;
            continue;
        }
        char clientIpStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIpStr, INET_ADDRSTRLEN);
        cout << "[INFO] Client connected from " << clientIpStr << ":" << ntohs(clientAddr.sin_port) << endl;
        client_threads.emplace_back(&Server::processClient, this, clientSocket);
        client_threads.erase(remove_if(client_threads.begin(), client_threads.end(),
                                       [](const thread &t)
                                       { return !t.joinable(); }),
                             client_threads.end());
        if (!client_threads.empty() && client_threads.back().joinable())
        {
            client_threads.back().detach();
        }
    }
}

// Handle a single client connection and process requests
void Server::processClient(int clientSocket)
{
    while (true)
    {
        uint32_t net_messageLength = 0;
        int lenRecv = recv(clientSocket, reinterpret_cast<char *>(&net_messageLength), sizeof(uint32_t), 0);
        if (lenRecv <= 0)
        {
            if (lenRecv == 0)
            {
                cout << "[INFO] Client gracefully disconnected while waiting for message length." << endl;
            }
            else
            {
                cerr << "[ERROR] Failed to receive message length from client. Error: " << get_socket_error() << endl;
            }
            break;
        }
        if (lenRecv != sizeof(uint32_t))
        {
            cerr << "[ERROR] Failed to receive the full message length. Expected " << sizeof(uint32_t) << " bytes, got " << lenRecv << " bytes." << endl;
            break;
        }
        uint32_t host_messageLength = ntohl(net_messageLength);
        if (host_messageLength == 0 || host_messageLength > 4096)
        {
            cerr << "[ERROR] Invalid message length received: " << host_messageLength << ". Must be > 0 and <= 4096." << endl;
            break;
        }
        vector<char> buffer(host_messageLength);
        int messageBodyBytesToReceive = static_cast<int>(host_messageLength);
        int bytesReceived = recv(clientSocket, buffer.data(), messageBodyBytesToReceive, MSG_WAITALL);
        if (bytesReceived <= 0)
        {
            if (bytesReceived == 0)
            {
                cout << "[INFO] Client gracefully disconnected after sending length, while waiting for message body." << endl;
            }
            else
            {
                cerr << "[ERROR] Failed to receive message body from client. Error: " << get_socket_error() << endl;
            }
            break;
        }
        if (bytesReceived != messageBodyBytesToReceive)
        {
            cerr << "[ERROR] Failed to receive the full message body. Expected " << messageBodyBytesToReceive << " bytes, got " << bytesReceived << " bytes." << endl;
            break;
        }
        string clientRequest(buffer.data(), bytesReceived);
        string command;
        vector<string> payloads;
        stringstream ss(clientRequest);
        string segment;
        if (getline(ss, command, '|'))
        {
            cout << "[REQUEST] Received Command: " << command;
            string payload_str;
            while (getline(ss, segment, '|'))
            {
                payloads.push_back(segment);
                payload_str += segment + (ss.peek() == EOF ? "" : "|");
            }
            if (!payload_str.empty())
            {
                cout << " | Payloads: " << payload_str;
            }
            cout << endl;
        }
        else
        {
            command = clientRequest;
            cout << "[REQUEST] Received Command (no delimiter): " << command << endl;
        }
        if (command == Protocol::LIST_PENDING_APPOINTMENTS)
        {
            handleListPendingAppointments(clientSocket);
        }
        else if (command == Protocol::ADMIN_APPROVE_APPOINTMENT)
        {
            if (payloads.size() >= 1)
            {
                handleAdminApproveAppointment(clientSocket, payloads[0]);
            }
            else
            {
                sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Missing appointment ID for approval");
            }
        }
        else if (command == Protocol::ADMIN_REJECT_APPOINTMENT)
        {
            if (payloads.size() >= 1)
            {
                handleAdminRejectAppointment(clientSocket, payloads[0]);
            }
            else
            {
                sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Missing appointment ID for rejection");
            }
        }
        if (command == Protocol::SIGN_IN)
        {
            if (payloads.size() >= 2)
            {
                handleSignIn(clientSocket, payloads[0], payloads[1]);
            }
            else
            {
                cerr << "[ERROR] Invalid SIGN_IN format from client. Request: " << clientRequest << endl;
                sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Invalid SIGN_IN format");
            }
        }
        else if (command == Protocol::SIGN_UP)
        {
            if (payloads.size() >= 2)
            {
                User newUser(payloads[0], payloads[1]);
                handleSignUp(clientSocket, newUser);
            }
            else
            {
                cerr << "[ERROR] Invalid SIGN_UP format from client. Request: " << clientRequest << endl;
                sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Invalid SIGN_UP format");
            }
        }
        else if (command == Protocol::REQUEST_APPOINTMENT)
        {
            if (payloads.size() >= 4)
            {
                string patientId = payloads[0];
                string doctorId = payloads[1];
                string dateTimeString = payloads[2];
                string details = payloads[3];
                string appointmentId = "APP" + to_string(appointments.size() + 1);
                Appointment newAppointment(appointmentId, patientId, doctorId, dateTimeString, details);
                handleAppointmentRequest(clientSocket, newAppointment);
            }
            else
            {
                cerr << "[ERROR] Invalid REQUEST_APPOINTMENT format from client. Expected PATIENT_ID|DOCTOR_ID|DATE_TIME|DETAILS. Request: " << clientRequest << endl;
                sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Invalid REQUEST_APPOINTMENT format. Expected: PATIENT_ID|DOCTOR_ID|DATE_TIME|DETAILS");
            }
        }
        else if (command == "GET_APPROVED_APPOINTMENTS")
        {
            handleGetApprovedAppointments(clientSocket);
        }
        else
        {
            cerr << "[WARNING] Unknown command from client: " << command << endl;
            sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Unknown command");
        }
    }
    closesocket(clientSocket);
    cout << "[INFO] Client socket closed." << endl;
}

// Handle sign-in request
void Server::handleSignIn(int clientSocket, const string &username, const string &password)
{
    cout << "[AUTH] Processing Sign-In for user: '" << username << "'" << endl;
    lock_guard<mutex> lock(g_users_mutex);
    auto it = users.find(username);
    if (it != users.end())
    {
        if (it->second.getPassword() == password)
        {
            sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Sign-in successful");
            cout << "[AUTH] User '" << username << "' signed in successfully." << endl;
        }
        else
        {
            sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Invalid credentials");
            cout << "[AUTH] Sign-in failed for user '" << username << "': Incorrect password." << endl;
        }
    }
    else
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|User not found");
        cout << "[AUTH] Sign-in failed: User '" << username << "' not found." << endl;
    }
}

// Handle sign-up request
void Server::handleSignUp(int clientSocket, const User &newUser)
{
    cout << "[AUTH] Processing Sign-Up for user: '" << newUser.getUsername() << "'" << endl;
    lock_guard<mutex> lock(g_users_mutex);
    if (this->users.count(newUser.getUsername()))
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Username already taken");
        cout << "[AUTH] Sign-Up failed for user '" << newUser.getUsername() << "': Username already taken." << endl;
    }
    else
    {
        this->users[newUser.getUsername()] = newUser;
        saveUser(newUser);
        sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Sign-up successful");
        cout << "[AUTH] User '" << newUser.getUsername() << "' registered successfully." << endl;
    }
}

// Handle appointment request
void Server::handleAppointmentRequest(int clientSocket, const Appointment &newAppointment)
{
    cout << "[APPOINTMENT] Processing Request for Patient: '" << newAppointment.getPatientName()
         << "', Doctor: '" << newAppointment.getDoctorName()
         << "', DateTime: '" << newAppointment.getAppointmentTime().toString()
         << "', Details: '" << newAppointment.getDetails() << "'" << endl;
    lock_guard<mutex> lock(g_appointments_mutex);
    appointments.push_back(newAppointment);
    cout << "[APPOINTMENT] Appointment ID '" << newAppointment.getid() << "' created and stored for patient '" << newAppointment.getPatientName() << "'." << endl;
    sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Appointment request received and registered with ID: " + newAppointment.getid());
}

// Approve appointment by ID
void Server::approveAppointment(int clientSocket, const string &appointmentId)
{
    cout << "[APPOINTMENT] Server-initiated Approval for Appointment ID: '" << appointmentId << "'" << endl;
    Admin adminCtrl;
    bool found = false;
    lock_guard<mutex> lock(g_appointments_mutex);
    for (auto &app : appointments)
    {
        if (app.getid() == appointmentId)
        {
            adminCtrl.approveAppointment(app);
            sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Appointment " + appointmentId + " approved by server.");
            cout << "[APPOINTMENT] Appointment ID '" << appointmentId << "' approved by server." << endl;
            found = true;
            break;
        }
    }
    if (!found)
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Appointment ID " + appointmentId + " not found for server approval.");
        cout << "[APPOINTMENT] Server approval failed: Appointment ID '" << appointmentId << "' not found." << endl;
    }
}

// Reject appointment by ID
void Server::rejectAppointment(int clientSocket, const string &appointmentId)
{
    cout << "[APPOINTMENT] Server-initiated Rejection for Appointment ID: '" << appointmentId << "'" << endl;
    Admin adminCtrl;
    bool found = false;
    lock_guard<mutex> lock(g_appointments_mutex);
    for (auto &app : appointments)
    {
        if (app.getid() == appointmentId)
        {
            adminCtrl.rejectAppointment(app);
            sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Appointment " + appointmentId + " rejected by server.");
            cout << "[APPOINTMENT] Appointment ID '" << appointmentId << "' rejected by server." << endl;
            found = true;
            break;
        }
    }
    if (!found)
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Appointment ID " + appointmentId + " not found for server rejection.");
        cout << "[APPOINTMENT] Server rejection failed: Appointment ID '" << appointmentId << "' not found." << endl;
    }
}

// List pending appointments for admin
void Server::handleListPendingAppointments(int clientSocket)
{
    cout << "[ADMIN_CMD] Listing pending appointments for client." << endl;
    string response_payload;
    bool first = true;
    lock_guard<mutex> lock(g_appointments_mutex);
    for (const auto &app : appointments)
    {
        if (app.getStatus().getStatus() == "pending")
        {
            if (!first)
                response_payload += ";";
            response_payload += app.getid() + "," + app.getPatientName() + "," + app.getDoctorName() + "," + app.getAppointmentTime().toString();
            first = false;
        }
    }
    if (response_payload.empty())
    {
        sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|No pending appointments.");
    }
    else
    {
        sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|" + response_payload);
    }
}

// Approve appointment via admin command
void Server::handleAdminApproveAppointment(int clientSocket, const string &appointmentId)
{
    cout << "[ADMIN_CMD] Client request to approve appointment ID: " << appointmentId << endl;
    Admin adminCtrl;
    bool found = false;
    lock_guard<mutex> lock(g_appointments_mutex);
    for (auto &app : appointments)
    {
        if (app.getid() == appointmentId)
        {
            adminCtrl.approveAppointment(app);
            sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Appointment " + appointmentId + " approved.");
            cout << "[ADMIN_CMD] Appointment " << appointmentId << " approved by client command." << endl;
            found = true;
            break;
        }
    }
    if (!found)
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Appointment ID " + appointmentId + " not found for approval.");
        cout << "[ADMIN_CMD] Approval failed for " << appointmentId << ": Not found." << endl;
    }
}

// Reject appointment via admin command
void Server::handleAdminRejectAppointment(int clientSocket, const string &appointmentId)
{
    cout << "[ADMIN_CMD] Client request to reject appointment ID: " << appointmentId << endl;
    Admin adminCtrl;
    bool found = false;
    lock_guard<mutex> lock(g_appointments_mutex);
    for (auto &app : appointments)
    {
        if (app.getid() == appointmentId)
        {
            adminCtrl.rejectAppointment(app);
            sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Appointment " + appointmentId + " rejected.");
            cout << "[ADMIN_CMD] Appointment " << appointmentId << " rejected by client command." << endl;
            found = true;
            break;
        }
    }
    if (!found)
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Appointment ID " + appointmentId + " not found for rejection.");
        cout << "[ADMIN_CMD] Rejection failed for " << appointmentId << ": Not found." << endl;
    }
}

// Send all appointments to viewer
void Server::handleGetApprovedAppointments(int clientSocket)
{
    cout << "[VIEWER_CMD] Request for approved appointments." << endl;
    string data;
    bool first = true;
    lock_guard<mutex> lock(g_appointments_mutex);
    for (const auto &app : appointments)
    {
        if (!first)
        {
            data += ";";
        }
        data += app.getid() + "," +
                app.getPatientName() + "," +
                app.getDoctorName() + "," +
                app.getAppointmentTime().toString() + "," +
                app.getStatus().getStatus() + "," +
                app.getDetails();
        first = false;
    }
    if (data.empty())
    {
        sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|No appointments found.");
    }
    else
    {
        sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|" + data);
    }
}

// Load users from file
void Server::loadUsers()
{
    ifstream inFile("users.txt");
    if (!inFile)
    {
        cout << "[INFO] No users.txt file found. Starting with an empty user database." << endl;
        return;
    }
    string line;
    int count = 0;
    while (getline(inFile, line))
    {
        istringstream iss(line);
        string username, password;
        if (getline(iss, username, '|') && getline(iss, password))
        {
            users[username] = User(username, password);
            count++;
        }
    }
    if (count > 0)
    {
        cout << "[INFO] Loaded " << count << " users from users.txt." << endl;
    }
    else
    {
        cout << "[INFO] users.txt was empty or contained no valid user entries." << endl;
    }
}

// Save a new user to file
void Server::saveUser(const User &newUser)
{
    ofstream outFile("users.txt", ios::app);
    if (!outFile)
    {
        cerr << "[ERROR] Failed to open users.txt for writing. User '" << newUser.getUsername() << "' not saved." << endl;
        return;
    }
    outFile << newUser.getUsername() << "|" << newUser.getPassword() << endl;
    cout << "[INFO] User '" << newUser.getUsername() << "' saved to users.txt." << endl;
}
