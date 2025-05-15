// src/viewer/ApprovedAppointmentsViewer.h
#ifndef APPROVED_APPOINTMENTS_VIEWER_H
#define APPROVED_APPOINTMENTS_VIEWER_H

#include "NetworkClient.h"
#include <string>
#include <vector>

using namespace std;

// Holds parsed appointment information for display
struct ApprovedAppointmentInfo
{
    string id;
    string patientName;
    string doctorName;
    string dateTime;
    string details;
};

// Viewer class to fetch and display all appointments from the server
class ApprovedAppointmentsViewer
{
public:
    // Initialize with server IP and port
    ApprovedAppointmentsViewer(const string &serverIp, int port);
    // Fetch and display all appointments
    void fetchAndDisplayAppointments();

private:
    NetworkClient networkClient;
    // Parse server data into appointment info structs
    vector<ApprovedAppointmentInfo> parseApprovedAppointments(const string &data);
};

#endif // APPROVED_APPOINTMENTS_VIEWER_H
