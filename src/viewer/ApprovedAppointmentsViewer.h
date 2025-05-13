// src/viewer/ApprovedAppointmentsViewer.h
#ifndef APPROVED_APPOINTMENTS_VIEWER_H
#define APPROVED_APPOINTMENTS_VIEWER_H

#include "NetworkClient.h" // Assuming NetworkClient.h is accessible via include paths
#include <string>
#include <vector>

// Struct to hold parsed appointment information
struct ApprovedAppointmentInfo
{
    std::string id;
    std::string patientName;
    std::string doctorName;
    std::string dateTime;
    std::string details;
};

class ApprovedAppointmentsViewer
{
public:
    ApprovedAppointmentsViewer(const std::string &serverIp, int port);
    void fetchAndDisplayAppointments();

private:
    NetworkClient networkClient;
    std::vector<ApprovedAppointmentInfo> parseApprovedAppointments(const std::string &data);
};

#endif // APPROVED_APPOINTMENTS_VIEWER_H
