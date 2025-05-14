// src/viewer/main.cpp
#include "ApprovedAppointmentsViewer.h"
#include <iostream>

using namespace std;

int main()
{
    // Replace with your server's IP and port if different
    string serverIp = "127.0.0.1";
    int port = 12345;

    ApprovedAppointmentsViewer viewer(serverIp, port);
    viewer.fetchAndDisplayAppointments();

    cout << "Press Enter to exit..." << endl;
    cin.get();
    return 0;
}
