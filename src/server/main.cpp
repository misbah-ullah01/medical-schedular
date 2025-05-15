// Entry point for the Medical Scheduler Server
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include "Server.h"
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif
using namespace std;

// Get the local IP address for display
string getLocalIPAddress()
{
    string ipAddress = "unknown";

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        return ipAddress;
    }

    char hostName[256];
    if (gethostname(hostName, sizeof(hostName)) != 0)
    {
        WSACleanup();
        return ipAddress;
    }

    struct addrinfo *result = NULL;
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(hostName, NULL, &hints, &result) == 0)
    {
        struct sockaddr_in *addr = (struct sockaddr_in *)result->ai_addr;
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(addr->sin_addr), ipStr, INET_ADDRSTRLEN);
        ipAddress = ipStr;
        freeaddrinfo(result);
    }

    WSACleanup();
#endif

    return ipAddress;
}

// Start the server in a background thread
void startServer(Server &server)
{
    server.start();
}

int main()
{
    // Display server info and start server
    string localIP = getLocalIPAddress();
    cout << "Starting Medical Scheduler Server..." << endl;
    cout << "Server IP Address: " << localIP << endl;
    cout << "Server Port: 12345" << endl;
    cout << "\nShare this IP address with clients to connect." << endl;

    Server server;
    thread serverThread(startServer, ref(server));

    // Admin CLI loop for appointment management
    while (true)
    {
        cout << "\n--- Admin Menu ---" << endl;
        cout << "1. List all appointments" << endl;
        cout << "2. Approve appointment by ID" << endl;
        cout << "3. Reject appointment by ID" << endl;
        cout << "4. Exit admin menu (server keeps running)" << endl;
        cout << "Enter choice: ";
        int choice;
        cin >> choice;
        cin.ignore();
        if (choice == 1)
        {
            // List all appointments
            cout << "\nAll Appointments:" << endl;
            for (const auto &app : server.getAppointments())
            {
                cout << "ID: " << app.getid() << ", Patient: " << app.getPatientName() << ", Doctor: " << app.getDoctorName()
                     << ", Time: " << app.getAppointmentTime().toString() << ", Status: " << app.getStatus().getStatus()
                     << ", Details: " << app.getDetails() << endl;
            }
        }
        else if (choice == 2)
        {
            // Approve appointment by ID
            cout << "Enter appointment ID to approve: ";
            string id;
            getline(cin, id);
            server.approveAppointment(-1, id);
        }
        else if (choice == 3)
        {
            // Reject appointment by ID
            cout << "Enter appointment ID to reject: ";
            string id;
            getline(cin, id);
            server.rejectAppointment(-1, id);
        }
        else if (choice == 4)
        {
            cout << "Exiting admin menu. Server will keep running in background." << endl;
            break;
        }
        else
        {
            cout << "Invalid choice. Try again." << endl;
        }
    }

    serverThread.join();
    return 0;
}