#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream> // For parsing
#include <cstdint> // For uint32_t
#include <fstream> // For loadUsers and saveUser

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // Link with Ws2_32.lib
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // For ntohl, htonl
#include <unistd.h>    // For close
#include <cstring>     // For memset, strerror
#endif
#include <thread>    // Added for multi-threading
#include <vector>    // Added for managing threads
#include <algorithm> // For std::find_if, std::remove_if
#include <mutex>     // Added for thread safety for shared resources (appointments, users)

#include "Server.h"
#include "Protocol.h" // Assuming Protocol::SIGN_IN, etc. and parsing functions are here
#include "User.h"
#include "Appointment.h"

// Helper to get error message
std::string get_socket_error()
{
#ifdef _WIN32
    return std::to_string(WSAGetLastError());
#else
    return strerror(errno);
#endif
}

std::mutex g_appointments_mutex; // Global mutex for appointments vector
std::mutex g_users_mutex;        // Global mutex for users map

// --- Server Member Function Definitions ---

Server::Server() : Server(12345) {} // Delegating constructor

Server::Server(int port) : port(port), serverSocket(INVALID_SOCKET)
{ // Initialize serverSocket
#ifdef _WIN32
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0)
    {
        std::cerr << "WSAStartup failed: " << wsaResult << std::endl;
        exit(EXIT_FAILURE);
    }
#endif

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << get_socket_error() << std::endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
#else
    if (serverSocket < 0)
    {
        std::cerr << "Socket creation failed: " << get_socket_error() << std::endl;
        exit(EXIT_FAILURE);
    }
#endif

    std::memset(&serverAddr, 0, sizeof(serverAddr)); // More portable
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

#ifdef _WIN32
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed: " << get_socket_error() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
#else
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Bind failed: " << get_socket_error() << std::endl;
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
#endif

#ifdef _WIN32
    if (listen(serverSocket, 3) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed: " << get_socket_error() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
#else
    if (listen(serverSocket, 3) < 0)
    {
        std::cerr << "Listen failed: " << get_socket_error() << std::endl;
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
#endif
    std::cout << "Server initialized on port " << port << std::endl;

    // Load persisted users
    loadUsers();
}

Server::~Server()
{
#ifdef _WIN32
    if (serverSocket != INVALID_SOCKET)
    {
        closesocket(serverSocket);
    }
    WSACleanup();
#else
    if (serverSocket >= 0)
    { // Check if socket descriptor is valid
        close(serverSocket);
    }
#endif
    std::cout << "Server shut down." << std::endl;
}

void Server::sendResponse(int clientSocket, const std::string &response)
{
    Protocol protocol;
    protocol.sendMessage(clientSocket, response);
    // std::cout << "Sent to client: " << response << std::endl; // Optional: Can be noisy
}

void Server::start()
{
    std::cout << "[INFO] Server listening on port " << port << "..." << std::endl;
    std::vector<std::thread> client_threads; // To store client threads

    while (true)
    {
        sockaddr_in clientAddr;
#ifdef _WIN32
        int clientAddrLen = sizeof(clientAddr);
#else
        socklen_t clientAddrLen = sizeof(clientAddr);
#endif
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen);

#ifdef _WIN32
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "[ERROR] Accept failed: " << get_socket_error() << std::endl;
            continue;
        }
#else
        if (clientSocket < 0)
        {
            std::cerr << "[ERROR] Accept failed: " << get_socket_error() << std::endl;
            continue;
        }
#endif
        char clientIpStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIpStr, INET_ADDRSTRLEN);
        std::cout << "[INFO] Client connected from " << clientIpStr << ":" << ntohs(clientAddr.sin_port) << std::endl;

        // Create a new thread to handle the client
        client_threads.emplace_back(&Server::processClient, this, clientSocket);
        // Detach the thread if you don't want to wait for it to finish (join).
        // For a server, detaching is common for client handlers.
        // However, be mindful of resource management if threads are detached.
        // For simplicity here, we'll let them run and potentially join later if needed,
        // or manage them in a more sophisticated way in a real application.
        // For now, let's detach to allow the main loop to continue accepting.
        // Clean up finished threads to prevent vector from growing indefinitely
        client_threads.erase(std::remove_if(client_threads.begin(), client_threads.end(),
                                            [](const std::thread &t)
                                            { return !t.joinable(); }),
                             client_threads.end());
        // Detach the latest thread
        if (!client_threads.empty() && client_threads.back().joinable())
        {
            client_threads.back().detach();
        }
    }
    // Optional: Join threads if they were not detached (e.g., on server shutdown)
    // for (auto& th : client_threads) {
    //     if (th.joinable()) {
    //         th.join();
    //     }
    // }
}

void Server::processClient(int clientSocket)
{
    // Loop to handle multiple requests from the same client
    while (true)
    {
        uint32_t net_messageLength = 0;
        // Receive the 4-byte message length in network byte order
        int lenRecv = recv(clientSocket, reinterpret_cast<char *>(&net_messageLength), sizeof(uint32_t), 0);

        if (lenRecv <= 0)
        {
            if (lenRecv == 0)
            {
                std::cout << "[INFO] Client gracefully disconnected while waiting for message length." << std::endl;
            }
            else
            {
                std::cerr << "[ERROR] Failed to receive message length from client. Error: " << get_socket_error() << std::endl;
            }
            break; // Exit loop on error or graceful disconnect
        }

        if (lenRecv != sizeof(uint32_t))
        {
            std::cerr << "[ERROR] Failed to receive the full message length. Expected " << sizeof(uint32_t) << " bytes, got " << lenRecv << " bytes." << std::endl;
            break; // Exit loop on partial length receive
        }

        // Convert message length from network byte order to host byte order
        uint32_t host_messageLength = ntohl(net_messageLength);

        // std::cout << "[DEBUG Server] Received net_messageLength: " << net_messageLength
        //           << ", converted to host_messageLength: " << host_messageLength << std::endl; // Removed

        // Basic validation for message length to prevent excessive memory allocation
        // Max length 4KB, adjust as needed. Also, 0 length is invalid.
        if (host_messageLength == 0 || host_messageLength > 4096)
        {
            std::cerr << "[ERROR] Invalid message length received: " << host_messageLength << ". Must be > 0 and <= 4096." << std::endl;
            // It might be good to send an error response to client before closing, if protocol allows
            // sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Invalid message length");
            break;
        }

        std::vector<char> buffer(host_messageLength); // Allocate buffer for message body only

        int messageBodyBytesToReceive = static_cast<int>(host_messageLength);
        int bytesReceived = recv(clientSocket, buffer.data(), messageBodyBytesToReceive, MSG_WAITALL);

        if (bytesReceived <= 0)
        {
            if (bytesReceived == 0)
            {
                std::cout << "[INFO] Client gracefully disconnected after sending length, while waiting for message body." << std::endl;
            }
            else
            {
                std::cerr << "[ERROR] Failed to receive message body from client. Error: " << get_socket_error() << std::endl;
            }
            break; // Exit loop
        }

        if (bytesReceived != messageBodyBytesToReceive)
        {
            std::cerr << "[ERROR] Failed to receive the full message body. Expected " << messageBodyBytesToReceive << " bytes, got " << bytesReceived << " bytes." << std::endl;
            break; // Exit loop on partial message body receive
        }

        std::string clientRequest(buffer.data(), bytesReceived); // Construct string with received bytes
        // std::cout << "Received from client (" << bytesReceived << " bytes): " << clientRequest << std::endl; // Replaced below

        std::string command;
        std::vector<std::string> payloads;
        std::stringstream ss(clientRequest);
        std::string segment;

        if (std::getline(ss, command, '|'))
        {
            std::cout << "[REQUEST] Received Command: " << command;
            std::string payload_str;
            while (std::getline(ss, segment, '|'))
            {
                payloads.push_back(segment);
                payload_str += segment + (ss.peek() == EOF ? "" : "|");
            }
            if (!payload_str.empty())
            {
                std::cout << " | Payloads: " << payload_str;
            }
            std::cout << std::endl;
        }
        else
        {
            command = clientRequest; // If no delimiter, the whole request is the command
            std::cout << "[REQUEST] Received Command (no delimiter): " << command << std::endl;
        }

        // Admin commands (simple check, ideally admin role should be verified)
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
        // TODO: Implement a LOGOUT command to allow graceful client-initiated disconnect
        // if (command == Protocol::LOGOUT_COMMAND) { // Assuming LOGOUT_COMMAND is defined
        //     sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Logout successful");
        //     std::cout << "[INFO] Client requested logout." << std::endl;
        //     break;
        // }

        if (command == Protocol::SIGN_IN)
        {
            if (payloads.size() >= 2)
            {
                handleSignIn(clientSocket, payloads[0], payloads[1]);
            }
            else
            {
                std::cerr << "[ERROR] Invalid SIGN_IN format from client. Request: " << clientRequest << std::endl;
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
                std::cerr << "[ERROR] Invalid SIGN_UP format from client. Request: " << clientRequest << std::endl;
                sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Invalid SIGN_UP format");
            }
        }
        else if (command == Protocol::REQUEST_APPOINTMENT)
        {
            if (payloads.size() >= 4)
            {
                std::string patientId = payloads[0];
                std::string doctorId = payloads[1];
                std::string dateTimeString = payloads[2];
                std::string details = payloads[3];

                std::string appointmentId = "APP" + std::to_string(appointments.size() + 1);
                Appointment newAppointment(appointmentId, patientId, doctorId, dateTimeString, details);
                handleAppointmentRequest(clientSocket, newAppointment);
            }
            else
            {
                std::cerr << "[ERROR] Invalid REQUEST_APPOINTMENT format from client. Expected PATIENT_ID|DOCTOR_ID|DATE_TIME|DETAILS. Request: " << clientRequest << std::endl;
                sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Invalid REQUEST_APPOINTMENT format. Expected: PATIENT_ID|DOCTOR_ID|DATE_TIME|DETAILS");
            }
        }
        else if (command == "GET_APPROVED_APPOINTMENTS") // New command for the viewer
        {
            handleGetApprovedAppointments(clientSocket);
        }
        else
        {
            std::cerr << "[WARNING] Unknown command from client: " << command << std::endl;
            sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Unknown command");
        }
    } // End of while(true) loop for handling multiple requests

#ifdef _WIN32
    closesocket(clientSocket);
#else
    close(clientSocket);
#endif
    std::cout << "[INFO] Client socket closed." << std::endl;
}

// --- Handler Function Definitions ---
void Server::handleSignIn(int clientSocket, const std::string &username, const std::string &password)
{
    std::cout << "[AUTH] Processing Sign-In for user: '" << username << "'" << std::endl;
    std::lock_guard<std::mutex> lock(g_users_mutex); // Lock users map
    auto it = users.find(username);
    if (it != users.end())
    {
        // User found, check password
        if (it->second.getPassword() == password)
        {
            sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Sign-in successful");
            std::cout << "[AUTH] User '" << username << "' signed in successfully." << std::endl;
        }
        else
        {
            sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Invalid credentials");
            std::cout << "[AUTH] Sign-in failed for user '" << username << "': Incorrect password." << std::endl;
        }
    }
    else
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|User not found");
        std::cout << "[AUTH] Sign-in failed: User '" << username << "' not found." << std::endl;
    }
}

void Server::handleSignUp(int clientSocket, const User &newUser)
{
    std::cout << "[AUTH] Processing Sign-Up for user: '" << newUser.getUsername() << "'" << std::endl;
    std::lock_guard<std::mutex> lock(g_users_mutex); // Lock users map
    if (this->users.count(newUser.getUsername()))
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Username already taken");
        std::cout << "[AUTH] Sign-Up failed for user '" << newUser.getUsername() << "': Username already taken." << std::endl;
    }
    else
    {
        this->users[newUser.getUsername()] = newUser;
        saveUser(newUser);
        sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Sign-up successful");
        std::cout << "[AUTH] User '" << newUser.getUsername() << "' registered successfully." << std::endl;
    }
}

void Server::handleAppointmentRequest(int clientSocket, const Appointment &newAppointment)
{
    std::cout << "[APPOINTMENT] Processing Request for Patient: '" << newAppointment.getPatientName()
              << "', Doctor: '" << newAppointment.getDoctorName()
              << "', DateTime: '" << newAppointment.getAppointmentTime().toString() // Use toString()
              << "', Details: '" << newAppointment.getDetails() << "'" << std::endl;

    std::lock_guard<std::mutex> lock(g_appointments_mutex); // Lock appointments vector
    appointments.push_back(newAppointment);

    std::cout << "[APPOINTMENT] Appointment ID '" << newAppointment.getid() << "' created and stored for patient '" << newAppointment.getPatientName() << "'." << std::endl;

    sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Appointment request received and registered with ID: " + newAppointment.getid());
}

void Server::approveAppointment(int clientSocket, const std::string &appointmentId)
{
    std::cout << "[APPOINTMENT] Server-initiated Approval for Appointment ID: '" << appointmentId << "'" << std::endl;
    Admin adminCtrl; // Create an Admin controller instance
    bool found = false;
    std::lock_guard<std::mutex> lock(g_appointments_mutex);
    for (auto &app : appointments)
    {
        if (app.getid() == appointmentId)
        {
            adminCtrl.approveAppointment(app);
            sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Appointment " + appointmentId + " approved by server.");
            std::cout << "[APPOINTMENT] Appointment ID '" << appointmentId << "' approved by server." << std::endl;
            found = true;
            // TODO: Persist appointment changes if needed
            break;
        }
    }
    if (!found)
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Appointment ID " + appointmentId + " not found for server approval.");
        std::cout << "[APPOINTMENT] Server approval failed: Appointment ID '" << appointmentId << "' not found." << std::endl;
    }
}

void Server::rejectAppointment(int clientSocket, const std::string &appointmentId)
{
    std::cout << "[APPOINTMENT] Server-initiated Rejection for Appointment ID: '" << appointmentId << "'" << std::endl;
    Admin adminCtrl;
    bool found = false;
    std::lock_guard<std::mutex> lock(g_appointments_mutex);
    for (auto &app : appointments)
    {
        if (app.getid() == appointmentId)
        {
            adminCtrl.rejectAppointment(app);
            sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Appointment " + appointmentId + " rejected by server.");
            std::cout << "[APPOINTMENT] Appointment ID '" << appointmentId << "' rejected by server." << std::endl;
            found = true;
            // TODO: Persist appointment changes if needed
            break;
        }
    }
    if (!found)
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Appointment ID " + appointmentId + " not found for server rejection.");
        std::cout << "[APPOINTMENT] Server rejection failed: Appointment ID '" << appointmentId << "' not found." << std::endl;
    }
}

// New handler functions for admin actions via client
void Server::handleListPendingAppointments(int clientSocket)
{
    std::cout << "[ADMIN_CMD] Listing pending appointments for client." << std::endl;
    std::string response_payload;
    bool first = true;
    std::lock_guard<std::mutex> lock(g_appointments_mutex);
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

void Server::handleAdminApproveAppointment(int clientSocket, const std::string &appointmentId)
{
    std::cout << "[ADMIN_CMD] Client request to approve appointment ID: " << appointmentId << std::endl;
    Admin adminCtrl;
    bool found = false;
    std::lock_guard<std::mutex> lock(g_appointments_mutex);
    for (auto &app : appointments)
    {
        if (app.getid() == appointmentId)
        {
            adminCtrl.approveAppointment(app);
            // TODO: Persist changes
            sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Appointment " + appointmentId + " approved.");
            std::cout << "[ADMIN_CMD] Appointment " << appointmentId << " approved by client command." << std::endl;
            found = true;
            break;
        }
    }
    if (!found)
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Appointment ID " + appointmentId + " not found for approval.");
        std::cout << "[ADMIN_CMD] Approval failed for " << appointmentId << ": Not found." << std::endl;
    }
}

void Server::handleAdminRejectAppointment(int clientSocket, const std::string &appointmentId)
{
    std::cout << "[ADMIN_CMD] Client request to reject appointment ID: " << appointmentId << std::endl;
    Admin adminCtrl;
    bool found = false;
    std::lock_guard<std::mutex> lock(g_appointments_mutex);
    for (auto &app : appointments)
    {
        if (app.getid() == appointmentId)
        {
            adminCtrl.rejectAppointment(app);
            // TODO: Persist changes
            sendResponse(clientSocket, Protocol::SUCCESS_MESSAGE + "|Appointment " + appointmentId + " rejected.");
            std::cout << "[ADMIN_CMD] Appointment " << appointmentId << " rejected by client command." << std::endl;
            found = true;
            break;
        }
    }
    if (!found)
    {
        sendResponse(clientSocket, Protocol::ERROR_MESSAGE + "|Appointment ID " + appointmentId + " not found for rejection.");
        std::cout << "[ADMIN_CMD] Rejection failed for " << appointmentId << ": Not found." << std::endl;
    }
}

void Server::handleGetApprovedAppointments(int clientSocket)
{
    std::cout << "[VIEWER_CMD] Request for approved appointments." << std::endl;
    std::string data;
    bool first = true;
    std::lock_guard<std::mutex> lock(g_appointments_mutex);
    for (const auto &app : appointments)
    {
        // Show all appointments regardless of status
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

// Load existing users from file
void Server::loadUsers()
{
    std::ifstream inFile("users.txt");
    if (!inFile)
    {
        std::cout << "[INFO] No users.txt file found. Starting with an empty user database." << std::endl;
        return;
    }
    std::string line;
    int count = 0;
    while (std::getline(inFile, line))
    {
        std::istringstream iss(line);
        std::string username, password;
        if (std::getline(iss, username, '|') && std::getline(iss, password))
        {
            users[username] = User(username, password);
            count++;
        }
    }
    if (count > 0)
    {
        std::cout << "[INFO] Loaded " << count << " users from users.txt." << std::endl;
    }
    else
    {
        std::cout << "[INFO] users.txt was empty or contained no valid user entries." << std::endl;
    }
}

// Append a new user to the file
void Server::saveUser(const User &newUser)
{
    std::ofstream outFile("users.txt", std::ios::app);
    if (!outFile)
    {
        std::cerr << "[ERROR] Failed to open users.txt for writing. User '" << newUser.getUsername() << "' not saved." << std::endl;
        return;
    }
    outFile << newUser.getUsername() << "|" << newUser.getPassword() << std::endl;
    std::cout << "[INFO] User '" << newUser.getUsername() << "' saved to users.txt." << std::endl;
}
