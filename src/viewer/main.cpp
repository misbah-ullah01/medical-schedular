// Entry point for the Approved Appointments Viewer application
#include "ApprovedAppointmentsViewer.h"
#include <iostream>

using namespace std;

int main()
{
    // Set server IP and port (change if needed)
    string serverIp = "127.0.0.1";
    int port = 12345;
    // Create viewer and fetch/display appointments
    ApprovedAppointmentsViewer viewer(serverIp, port);
    viewer.fetchAndDisplayAppointments();
    cout << "Press Enter to exit..." << endl;
    cin.get();
    return 0;
}
