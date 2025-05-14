// src/viewer/ApprovedAppointmentsViewer.cpp
#include "ApprovedAppointmentsViewer.h"
#include "Protocol.h" // For command strings and message constants
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

struct ApprovedAppointmentInfo
{
    string id;
    string patientName;
    string doctorName;
    string dateTime;
    string status;
    string details;
};

ApprovedAppointmentsViewer::ApprovedAppointmentsViewer(const string &serverIp, int port)
    : networkClient(serverIp, port) {} // Assumes NetworkClient.h is in include path for ApprovedAppointmentsViewer.h

// Parses the raw data string from the server into a vector of ApprovedAppointmentInfo structs
vector<ApprovedAppointmentInfo> ApprovedAppointmentsViewer::parseApprovedAppointments(const string &data)
{
    vector<ApprovedAppointmentInfo> appointments;
    if (data.empty() || data == "No appointments found.")
    {
        return appointments; // Return empty vector if no data or specific message
    }

    stringstream ss(data);
    string appointment_record_str;

    // Appointments are separated by ';'
    while (getline(ss, appointment_record_str, ';'))
    {
        if (appointment_record_str.empty())
            continue; // Skip empty segments

        stringstream item_ss(appointment_record_str);
        string id, patient, doctor, datetime, status, details;

        // Fields within each appointment record are separated by ','
        // Format: ID,Patient,Doctor,DateTime,Status,Details
        if (getline(item_ss, id, ',') &&
            getline(item_ss, patient, ',') &&
            getline(item_ss, doctor, ',') &&
            getline(item_ss, datetime, ',') &&
            getline(item_ss, status, ',') &&
            getline(item_ss, details, ','))
        { // Ensure all fields are read
            appointments.push_back({id, patient, doctor, datetime, status, details});
        }
        else
        {
            cerr << "[Viewer] Warning: Could not parse appointment record: " << appointment_record_str << endl;
        }
    }
    return appointments;
}

void ApprovedAppointmentsViewer::fetchAndDisplayAppointments()
{
    if (!networkClient.connectToServer())
    {
        cerr << "[Viewer] Error: Failed to connect to server." << endl;
        return;
    }
    cout << "[Viewer] Connected to server. Fetching all appointments..." << endl;

    string request = Protocol::GET_APPROVED_APPOINTMENTS;
    networkClient.sendRequest(request);                // FIX: use sendRequest
    string response = networkClient.receiveResponse(); // FIX: use receiveResponse
    networkClient.disconnect();                        // FIX: use disconnect

    cout << "[Viewer] Raw response from server: " << response << endl; // Debugging line

    stringstream ss_response(response);
    string command_part;
    string data_part;

    if (!getline(ss_response, command_part, '|'))
    {
        cerr << "[Viewer] Error: Invalid response format from server (missing '|'): " << response << endl;
        return;
    }
    // The rest of the stream is the data_part
    getline(ss_response, data_part);

    if (command_part == Protocol::SUCCESS_MESSAGE)
    {
        if (data_part == "No appointments found." || data_part.empty())
        {
            cout << "[Viewer] No appointments to display." << endl;
        }
        else
        {
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
