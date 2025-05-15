// Shows all approved, pending, and rejected appointments by connecting to the server
#include "ApprovedAppointmentsViewer.h"
#include "Protocol.h"
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

// Struct to hold appointment info (should match server's output format)
struct ApprovedAppointmentInfo
{
    string id;
    string patientName;
    string doctorName;
    string dateTime;
    string status;
    string details;
};

// Constructor: initializes network client with server IP and port
ApprovedAppointmentsViewer::ApprovedAppointmentsViewer(const string &serverIp, int port)
    : networkClient(serverIp, port) {}

// Parses the server's response string into a vector of appointment info structs
vector<ApprovedAppointmentInfo> ApprovedAppointmentsViewer::parseApprovedAppointments(const string &data)
{
    vector<ApprovedAppointmentInfo> appointments;
    if (data.empty() || data == "No appointments found.")
    {
        return appointments;
    }
    stringstream ss(data);
    string appointment_record_str;
    // Each appointment is separated by ';'
    while (getline(ss, appointment_record_str, ';'))
    {
        if (appointment_record_str.empty())
            continue;
        stringstream item_ss(appointment_record_str);
        string id, patient, doctor, datetime, status, details;
        // Each field is separated by ',')
        if (getline(item_ss, id, ',') &&
            getline(item_ss, patient, ',') &&
            getline(item_ss, doctor, ',') &&
            getline(item_ss, datetime, ',') &&
            getline(item_ss, status, ',') &&
            getline(item_ss, details, ','))
        {
            appointments.push_back({id, patient, doctor, datetime, status, details});
        }
        else
        {
            cerr << "[Viewer] Warning: Could not parse appointment record: " << appointment_record_str << endl;
        }
    }
    return appointments;
}

// Connects to the server, fetches all appointments, and displays them
void ApprovedAppointmentsViewer::fetchAndDisplayAppointments()
{
    // Attempt to connect to the server
    if (!networkClient.connectToServer())
    {
        cerr << "[Viewer] Error: Failed to connect to server." << endl;
        return;
    }
    cout << "[Viewer] Connected to server. Fetching all appointments..." << endl;
    // Send request for all appointments
    string request = Protocol::GET_APPROVED_APPOINTMENTS;
    networkClient.sendRequest(request);
    string response = networkClient.receiveResponse();
    networkClient.disconnect();
    cout << "[Viewer] Raw response from server: " << response << endl;
    // Parse the response: first part is status, second part is data
    stringstream ss_response(response);
    string command_part;
    string data_part;
    if (!getline(ss_response, command_part, '|'))
    {
        cerr << "[Viewer] Error: Invalid response format from server (missing '|'): " << response << endl;
        return;
    }
    getline(ss_response, data_part);
    if (command_part == Protocol::SUCCESS_MESSAGE)
    {
        if (data_part == "No appointments found." || data_part.empty())
        {
            cout << "[Viewer] No appointments to display." << endl;
        }
        else
        {
            // Parse and display each appointment
            vector<ApprovedAppointmentInfo> appointments = parseApprovedAppointments(data_part);
            if (appointments.empty())
            {
                cout << "[Viewer] No appointments to display (data might be empty or unparseable): " << data_part << endl;
            }
            else
            {
                cout << "\n--- All Appointments ---" << endl;
                for (const auto &app : appointments)
                {
                    cout << "ID:        " << app.id << endl;
                    cout << "  Patient: " << app.patientName << endl;
                    cout << "  Doctor:  " << app.doctorName << endl;
                    cout << "  Time:    " << app.dateTime << endl;
                    cout << "  Status:  " << app.status << endl;
                    cout << "  Details: " << app.details << endl;
                    cout << "---------------------------" << endl;
                }
            }
        }
    }
    else if (command_part == Protocol::ERROR_MESSAGE)
    {
        cerr << "[Viewer] Server Error: " << data_part << endl;
    }
    else
    {
        cerr << "[Viewer] Error: Unexpected response command from server: " << command_part << endl;
        cerr << "[Viewer] Full response: " << response << endl;
    }
}
