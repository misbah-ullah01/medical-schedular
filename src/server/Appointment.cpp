// filepath: d:\GIKI (CS)\Semester 2\CS112\Semester Project\Project 1\medical-scheduler\src\server\Appointment.cpp
#include "Appointment.h"
#include <cstdio> // For sscanf. Consider <sstream> for more robust parsing.
#include <string> // Required for string

using namespace std;

// Constructor: Appointment(const string& patName, const string& docName, const timezone& appTime, const string& appID)
Appointment::Appointment(const string &patName, const string &docName, const timezone &appTime, const string &appID)
    : id(appID), patientName(patName), doctorName(docName), appointmentTime(appTime), status(), details("")
{
    // status() will call AppointmentStatus default constructor ("pending")
}

// Constructor: Appointment(const string& appointmentId, const string& patientName, const string& doctorName, const string& dateTimeString, const string& appDetails)
Appointment::Appointment(const string &appointmentId, const string &patName, const string &docName, const string &dateTimeString, const string &appDetails)
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
string Appointment::getid() const
{
    return id;
}

string Appointment::getPatientName() const
{
    return patientName;
}

string Appointment::getDoctorName() const
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

string Appointment::getDetails() const
{
    return details;
}

// Setters
void Appointment::setStatus(const AppointmentStatus &newStatus)
{
    status = newStatus;
}