// src/viewer/ApprovedAppointmentsViewer.h
#ifndef APPROVED_APPOINTMENTS_VIEWER_H
#define APPROVED_APPOINTMENTS_VIEWER_H

#include "NetworkClient.h" // Assuming NetworkClient.h is accessible via include paths
#include <string>
#include <vector>

using namespace std;

// Struct to hold parsed appointment information
struct ApprovedAppointmentInfo
{
    string id;
    string patientName;
    string doctorName;
    string dateTime;
    string details;
};

class ApprovedAppointmentsViewer
{
public:
    ApprovedAppointmentsViewer(const string &serverIp, int port);
    void fetchAndDisplayAppointments();

private:
    NetworkClient networkClient;
    vector<ApprovedAppointmentInfo> parseApprovedAppointments(const string &data);
};

#endif // APPROVED_APPOINTMENTS_VIEWER_H
