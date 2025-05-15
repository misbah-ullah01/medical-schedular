#pragma once
#ifndef ADMIN_H
#define ADMIN_H

#include <iostream>
#include <vector>
#include "Appointment.h"

using namespace std;

// Handles admin actions for appointments
class Admin
{
private:
    vector<Appointment> appointments; // List of appointments managed by the admin

public:
    Admin();
    // Approve an appointment
    void approveAppointment(Appointment &appointment);
    // Reject an appointment
    void rejectAppointment(Appointment &appointment);
    // View all pending appointments
    vector<Appointment> viewPendingAppointments(const vector<Appointment> &appointments);
};
#endif // ADMIN_H