#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <string>
#include <ctime>
#include <iostream>

using namespace std;

struct timezone
{
public:
    // Date and time components
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
        return false; // Ensure all paths return a value
    }
    bool operator>(const timezone &other) const
    {
        return other < *this;
    }

    // New toString method
    std::string toString() const
    {
        char buffer[20]; // "YYYY-MM-DD HH:MM"
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d", year, month, day, hour, minute);
        return std::string(buffer);
    }
};

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

    // Add stream operator as a friend function
    friend ostream &operator<<(ostream &os, const AppointmentStatus &status)
    {
        os << status.getStatus();
        return os;
    }
};

class Appointment
{
protected:
    std::string id; // Changed from int to std::string
    std::string patientName;
    std::string doctorName;
    timezone appointmentTime;
    AppointmentStatus status; // Default constructor of AppointmentStatus sets to "pending"
    std::string details;      // Added details member

public:
    // Existing constructor - signature updated for id type
    Appointment(const std::string &patName, const std::string &docName, const timezone &appTime, const std::string &appID);

    // New constructor for server usage (dateTimeString to be parsed)
    Appointment(const std::string &appointmentId, const std::string &patientName, const std::string &doctorName, const std::string &dateTimeString, const std::string &appDetails);

    // Default constructor (optional, but can be useful)
    Appointment();

    // Copy constructor - ensure all members are copied
    Appointment(const Appointment &other);
    // Assignment operator - ensure all members are assigned
    Appointment &operator=(const Appointment &other);

    // Getters - id return type updated, new getter for details
    std::string getid() const;
    std::string getPatientName() const;
    std::string getDoctorName() const;
    timezone getAppointmentTime() const; // Added getter
    AppointmentStatus getStatus() const; // Added getter
    std::string getDetails() const;
    std::string getId() const { return id; } // Added getter

    // Setters (optional, add as needed)
    void setStatus(const AppointmentStatus &newStatus); // Example setter
};

#endif // APPOINTMENT_H