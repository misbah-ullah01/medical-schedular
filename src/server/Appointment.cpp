// filepath: d:\GIKI (CS)\Semester 2\CS112\Semester Project\Project 1\medical-scheduler\src\server\Appointment.cpp
#include "Appointment.h"
#include <cstdio> // For sscanf. Consider <sstream> for more robust parsing.
#include <string> // Required for std::string

// Constructor: Appointment(const std::string& patName, const std::string& docName, const timezone& appTime, const std::string& appID)
Appointment::Appointment(const std::string &patName, const std::string &docName, const timezone &appTime, const std::string &appID)
    : id(appID), patientName(patName), doctorName(docName), appointmentTime(appTime), status(), details("")
{
    // status() will call AppointmentStatus default constructor ("pending")
}

// Constructor: Appointment(const std::string& appointmentId, const std::string& patientName, const std::string& doctorName, const std::string& dateTimeString, const std::string& appDetails)
Appointment::Appointment(const std::string &appointmentId, const std::string &patName, const std::string &docName, const std::string &dateTimeString, const std::string &appDetails)
    : id(appointmentId), patientName(patName), doctorName(docName), details(appDetails), status()
{ // status() will call AppointmentStatus default constructor
    // Parse dateTimeString "YYYY-MM-DD HH:MM" into timezone struct
    // Basic parsing with sscanf. Error checking should be added for production code.
    // Ensure appointmentTime.second is handled (e.g., defaulted to 0 by timezone constructor if not parsed)
    sscanf(dateTimeString.c_str(), "%d-%d-%d %d:%d",
           &appointmentTime.year, &appointmentTime.month, &appointmentTime.day,
           &appointmentTime.hour, &appointmentTime.minute);
}

// Default constructor
Appointment::Appointment()
    : id(""), patientName(""), doctorName(""), appointmentTime(), status(), details("")
{
    // Default constructors for timezone and AppointmentStatus will be called.
}

// Copy constructor
Appointment::Appointment(const Appointment &other)
    : id(other.id),
      patientName(other.patientName),
      doctorName(other.doctorName),
      appointmentTime(other.appointmentTime),
      status(other.status),
      details(other.details) {}

// Assignment operator
Appointment &Appointment::operator=(const Appointment &other)
{
    if (this != &other)
    {
        id = other.id;
        patientName = other.patientName;
        doctorName = other.doctorName;
        appointmentTime = other.appointmentTime;
        status = other.status;
        details = other.details;
    }
    return *this;
}

// Getters
std::string Appointment::getid() const
{
    return id;
}

std::string Appointment::getPatientName() const
{
    return patientName;
}

std::string Appointment::getDoctorName() const
{
    return doctorName;
}

timezone Appointment::getAppointmentTime() const
{
    return appointmentTime;
}

AppointmentStatus Appointment::getStatus() const
{
    return status;
}

std::string Appointment::getDetails() const
{
    return details;
}

// Setters
void Appointment::setStatus(const AppointmentStatus &newStatus)
{
    status = newStatus;
}