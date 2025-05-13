// src/viewer/main.cpp
#include "ApprovedAppointmentsViewer.h"
#include <iostream>

int main()
{
    // Replace with your server's IP and port if different
    std::string serverIp = "127.0.0.1";
    int port = 12345;

    ApprovedAppointmentsViewer viewer(serverIp, port);
    viewer.fetchAndDisplayAppointments();

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    return 0;
}
