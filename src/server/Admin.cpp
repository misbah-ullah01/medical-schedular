// Implements Admin class methods
#include "Admin.h"
#include "Appointment.h"
#include <iostream>
#include <vector>

using namespace std;

Admin::Admin() {}

// Approve an appointment
void Admin::approveAppointment(Appointment &appointment)
{
    AppointmentStatus newStatus("approved");
    appointment.setStatus(newStatus);
    cout << "Appointment approved: " << appointment.getStatus().getStatus() << endl;
}

// Reject an appointment
void Admin::rejectAppointment(Appointment &appointment)
{
    AppointmentStatus newStatus("rejected");
    appointment.setStatus(newStatus);
    cout << "Appointment rejected: " << appointment.getStatus().getStatus() << endl;
}

// Return all pending appointments
vector<Appointment> Admin::viewPendingAppointments(const vector<Appointment> &appointments)
{
    vector<Appointment> pendingAppointments;
    AppointmentStatus pending("pending");
    for (const auto &appointment : appointments)
    {
        if (appointment.getStatus() == pending)
        {
            pendingAppointments.push_back(appointment);
        }
    }
    return pendingAppointments;
}
