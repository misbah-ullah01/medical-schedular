#pragma once
#ifndef ADMIN_H
#define ADMIN_H

#include <iostream>
#include <vector>
#include "Appointment.h"

using namespace std;

class Admin
{
private:
    vector<Appointment> appointments;

public:
    Admin();
    void approveAppointment(Appointment &appointment);
    void rejectAppointment(Appointment &appointment);
    void rejectAppointment(const string &appointmentId); // Changed from int to string
    vector<Appointment> viewPendingAppointments(const vector<Appointment> &appointments);
    void viewAppointments();
};
#endif // ADMIN_H