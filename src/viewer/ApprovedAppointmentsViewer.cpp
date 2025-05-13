// src/viewer/ApprovedAppointmentsViewer.cpp
#include "ApprovedAppointmentsViewer.h"
#include "Protocol.h" // For command strings and message constants
#include <iostream>
#include <sstream>
#include <vector>

struct ApprovedAppointmentInfo
{
    std::string id;
    std::string patientName;
    std::string doctorName;
    std::string dateTime;
    std::string status;
    std::string details;
};

ApprovedAppointmentsViewer::ApprovedAppointmentsViewer(const std::string &serverIp, int port)
    : networkClient(serverIp, port) {} // Assumes NetworkClient.h is in include path for ApprovedAppointmentsViewer.h

// Parses the raw data string from the server into a vector of ApprovedAppointmentInfo structs
std::vector<ApprovedAppointmentInfo> ApprovedAppointmentsViewer::parseApprovedAppointments(const std::string &data)
{
    std::vector<ApprovedAppointmentInfo> appointments;
    if (data.empty() || data == "No appointments found.")
    {
        return appointments; // Return empty vector if no data or specific message
    }

    std::stringstream ss(data);
    std::string appointment_record_str;

    // Appointments are separated by ';'
    while (std::getline(ss, appointment_record_str, ';'))
    {
        if (appointment_record_str.empty())
            continue; // Skip empty segments

        std::stringstream item_ss(appointment_record_str);
        std::string id, patient, doctor, datetime, status, details;

        // Fields within each appointment record are separated by ','
        // Format: ID,Patient,Doctor,DateTime,Status,Details
        if (std::getline(item_ss, id, ',') &&
            std::getline(item_ss, patient, ',') &&
            std::getline(item_ss, doctor, ',') &&
            std::getline(item_ss, datetime, ',') &&
            std::getline(item_ss, status, ',') &&
            std::getline(item_ss, details, ','))
        { // Ensure all fields are read
            appointments.push_back({id, patient, doctor, datetime, status, details});
        }
        else
        {
            std::cerr << "[Viewer] Warning: Could not parse appointment record: " << appointment_record_str << std::endl;
        }
    }
    return appointments;
}

void ApprovedAppointmentsViewer::fetchAndDisplayAppointments()
{
    if (!networkClient.connectToServer())
    {
        std::cerr << "[Viewer] Error: Failed to connect to server." << std::endl;
        return;
    }
    std::cout << "[Viewer] Connected to server. Fetching all appointments..." << std::endl;

    std::string request = Protocol::GET_APPROVED_APPOINTMENTS;
    networkClient.sendRequest(request);                     // FIX: use sendRequest
    std::string response = networkClient.receiveResponse(); // FIX: use receiveResponse
    networkClient.disconnect();                             // FIX: use disconnect

    std::cout << "[Viewer] Raw response from server: " << response << std::endl; // Debugging line

    std::stringstream ss_response(response);
    std::string command_part;
    std::string data_part;

    if (!std::getline(ss_response, command_part, '|'))
    {
        std::cerr << "[Viewer] Error: Invalid response format from server (missing '|'): " << response << std::endl;
        return;
    }
    // The rest of the stream is the data_part
    std::getline(ss_response, data_part);

    if (command_part == Protocol::SUCCESS_MESSAGE)
    {
        if (data_part == "No appointments found." || data_part.empty())
        {
            std::cout << "[Viewer] No appointments to display." << std::endl;
        }
        else
        {
            std::vector<ApprovedAppointmentInfo> appointments = parseApprovedAppointments(data_part);
            if (appointments.empty())
            {
                std::cout << "[Viewer] No appointments to display (data might be empty or unparseable): " << data_part << std::endl;
            }
            else
            {
                std::cout << "\n--- All Appointments ---" << std::endl;
                for (const auto &app : appointments)
                {
                    std::cout << "ID:        " << app.id << std::endl;
                    std::cout << "  Patient: " << app.patientName << std::endl;
                    std::cout << "  Doctor:  " << app.doctorName << std::endl;
                    std::cout << "  Time:    " << app.dateTime << std::endl;
                    std::cout << "  Status:  " << app.status << std::endl;
                    std::cout << "  Details: " << app.details << std::endl;
                    std::cout << "---------------------------" << std::endl;
                }
            }
        }
    }
    else if (command_part == Protocol::ERROR_MESSAGE)
    {
        std::cerr << "[Viewer] Server Error: " << data_part << std::endl;
    }
    else
    {
        std::cerr << "[Viewer] Error: Unexpected response command from server: " << command_part << std::endl;
        std::cerr << "[Viewer] Full response: " << response << std::endl;
    }
}
