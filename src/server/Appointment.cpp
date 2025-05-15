// Implements Appointment, timezone, and AppointmentStatus methods
#include "Appointment.h"
#include <cstdio>
#include <string>

using namespace std;

// Create appointment with patient, doctor, time, and ID
Appointment::Appointment(const string &patName, const string &docName, const timezone &appTime, const string &appID)
    : id(appID), patientName(patName), doctorName(docName), appointmentTime(appTime), status(), details("")
{
}

// Create appointment from string date/time and details
Appointment::Appointment(const string &appointmentId, const string &patName, const string &docName, const string &dateTimeString, const string &appDetails)
    : id(appointmentId), patientName(patName), doctorName(docName), details(appDetails), status()
{
    sscanf(dateTimeString.c_str(), "%d-%d-%d %d:%d",
           &appointmentTime.year, &appointmentTime.month, &appointmentTime.day,
           &appointmentTime.hour, &appointmentTime.minute);
}

// Default constructor
Appointment::Appointment()
    : id(""), patientName(""), doctorName(""), appointmentTime(), status(), details("")
{
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

// Set appointment status
void Appointment::setStatus(const AppointmentStatus &newStatus)
{
    status = newStatus;
}