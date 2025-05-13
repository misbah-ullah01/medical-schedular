#include "Admin.h"
#include "Appointment.h"
#include <iostream>
#include <vector>

using namespace std;

Admin::Admin() {}

void Admin::approveAppointment(Appointment &appointment)
{
    AppointmentStatus newStatus("approved");
    appointment.setStatus(newStatus);
    cout << "Appointment approved: " << appointment.getStatus().getStatus() << endl;
}

void Admin::rejectAppointment(Appointment &appointment)
{
    AppointmentStatus newStatus("rejected");
    appointment.setStatus(newStatus);
    cout << "Appointment rejected: " << appointment.getStatus().getStatus() << endl;
}

void Admin::rejectAppointment(const string &appointmentId)
{
    for (auto &x : appointments)
    {
        if (x.getId() == appointmentId)
        {
            rejectAppointment(x);
            break;
        }
    }
}

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

void Admin::viewAppointments()
{
    for (const auto &appointment : appointments)
    {
        cout << "Appointment Id: " << appointment.getId() << ", Status: " << appointment.getStatus().getStatus() << endl;
    }
}
