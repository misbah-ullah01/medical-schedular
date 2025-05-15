#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <string>
#include <ctime>
#include <iostream>

using namespace std;

// Represents a date and time for an appointment
struct timezone
{
    int year, month, day, hour, minute;
    timezone(int y, int m, int d, int h, int min, int s = 0) : year(y), month(m), day(d), hour(h), minute(min) {}
    timezone() : year(0), month(0), day(0), hour(0), minute(0) {}
    timezone(const timezone &other) : year(other.year), month(other.month), day(other.day), hour(other.hour), minute(other.minute) {}
    timezone &operator=(const timezone &other)
    {
        if (this != &other)
        {
            year = other.year;
            month = other.month;
            day = other.day;
            hour = other.hour;
            minute = other.minute;
        }
        return *this;
    }
    bool operator==(const timezone &other) const
    {
        return year == other.year && month == other.month && day == other.day &&
               hour == other.hour && minute == other.minute;
    }
    bool operator!=(const timezone &other) const
    {
        return !(*this == other);
    }
    bool operator<(const timezone &other) const
    {
        if (year != other.year)
            return year < other.year;
        if (month != other.month)
            return month < other.month;
        if (day != other.day)
            return day < other.day;
        if (hour != other.hour)
            return hour < other.hour;
        if (minute != other.minute)
            return minute < other.minute;
        return false;
    }
    bool operator>(const timezone &other) const
    {
        return other < *this;
    }
    // Convert to string for display
    string toString() const
    {
        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d", year, month, day, hour, minute);
        return string(buffer);
    }
};

// Represents the status of an appointment
struct AppointmentStatus
{
private:
    string status;

public:
    AppointmentStatus(const string &s) : status(s) {}
    AppointmentStatus() : status("pending") {}
    AppointmentStatus(const AppointmentStatus &other) : status(other.status) {}
    AppointmentStatus &operator=(const AppointmentStatus &other)
    {
        if (this != &other)
        {
            status = other.status;
        }
        return *this;
    }
    bool operator==(const AppointmentStatus &other) const
    {
        return status == other.status;
    }
    string getStatus() const
    {
        return status;
    }
    void setStatus(const string &newStatus)
    {
        status = newStatus;
    }
    void approve()
    {
        status = "approved";
    }
    void reject()
    {
        status = "rejected";
    }
    friend ostream &operator<<(ostream &os, const AppointmentStatus &status)
    {
        os << status.getStatus();
        return os;
    }
};

// Represents a medical appointment
class Appointment
{
protected:
    string id;
    string patientName;
    string doctorName;
    timezone appointmentTime;
    AppointmentStatus status;
    string details;

public:
    Appointment(const string &patName, const string &docName, const timezone &appTime, const string &appID);
    Appointment(const string &appointmentId, const string &patientName, const string &doctorName, const string &dateTimeString, const string &appDetails);
    Appointment();
    Appointment(const Appointment &other);
    Appointment &operator=(const Appointment &other);
    // Getters
    string getid() const;
    string getPatientName() const;
    string getDoctorName() const;
    timezone getAppointmentTime() const;
    AppointmentStatus getStatus() const;
    string getDetails() const;
    string getId() const { return id; }
    // Setters
    void setStatus(const AppointmentStatus &newStatus);
};

#endif // APPOINTMENT_H