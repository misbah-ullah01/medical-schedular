#include <iostream>
#include <limits>
#include "Patient.h"
#include "NetworkClient.h"
#include <string>
#include <stdexcept>

using namespace std;

void pauseConsole()
{
    cout << "\nPress Enter to continue...";
    string dummy;
    getline(cin, dummy);
}

void showPatientMenu(Patient &patient)
{
    while (true)
    {
        cout << "\nPatient Menu for " << patient.getUsername() << ":\n";
        cout << "1. Request Appointment\n";
        cout << "2. View My Appointments\n";
        cout << "3. Logout\n";
        cout << "Enter your choice (1-3): ";

        string input;
        getline(cin, input);

        int choice;
        try
        {
            if (input.empty())
            {
                cout << "Please enter a number between 1 and 3.\n";
                continue;
            }
            choice = stoi(input);
        }
        catch (...)
        {
            cout << "Invalid input. Please enter a number between 1 and 3.\n";
            continue;
        }

        switch (choice)
        {
        case 1:
            patient.requestAppointment("");
            break;
        case 2:
            cout << "View appointments functionality coming soon...\n";
            break;
        case 3:
            cout << "Logging out...\n";
            return;
        default:
            cout << "Invalid choice. Please enter a number between 1 and 3.\n";
        }
    }
}

void showMainMenu(Patient &patient)
{
    while (true)
    {
        cout << "\nMain Menu:\n";
        cout << "1. Sign Up\n2. Sign In\n3. Exit\n";
        cout << "Enter your choice (1-3): ";

        string input;
        getline(cin, input);

        int choice;
        try
        {
            if (input.empty())
            {
                cout << "Please enter a number between 1 and 3.\n";
                continue;
            }
            choice = stoi(input);
        }
        catch (...)
        {
            cout << "Invalid input. Please enter a number between 1 and 3.\n";
            continue;
        }

        switch (choice)
        {
        case 1:
        {
            if (patient.signUp())
            {
                showPatientMenu(patient);
            }
            break;
        }
        case 2:
        {
            if (patient.signIn())
            {
                showPatientMenu(patient);
            }
            break;
        }
        case 3:
            cout << "Thank you for using Medical Scheduler. Goodbye!" << endl;
            return;
        default:
            cout << "Invalid choice. Please enter a number between 1 and 3.\n";
        }
    }
}

int main()
{
    try
    {
        cout << "Welcome to the Medical Scheduler!" << endl;

        string serverIP;
        cout << "Enter server IP address (or press Enter for default 127.0.0.1): ";
        getline(cin, serverIP);
        if (serverIP.empty())
        {
            serverIP = "127.0.0.1"; // localhost as default
        }

        cout << "Attempting to connect to " << serverIP << "..." << endl;
        NetworkClient networkClient(serverIP, 12345);

        if (!networkClient.connectToServer())
        {
            cout << "Failed to connect to server at " << serverIP << endl;
            cout << "Please check if:" << endl;
            cout << "1. The server is running" << endl;
            cout << "2. The IP address is correct" << endl;
            cout << "3. Your firewall is not blocking the connection" << endl;
            pauseConsole();
            return 1;
        }
        cout << "Connected to server at " << serverIP << endl;

        Patient patient(&networkClient); // Pass the already connected client
        showMainMenu(patient);
        pauseConsole();
        return 0;
    }
    catch (const exception &e)
    {
        cout << "An error occurred: " << e.what() << endl;
        pauseConsole();
        return 1;
    }
    catch (...)
    {
        cout << "An unknown error occurred" << endl;
        pauseConsole();
        return 1;
    }
}