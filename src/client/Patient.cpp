#include "Patient.h"
#include "NetworkClient.h"
#include "VigenereCipher.h"
#include <iostream>
#include <string>

using namespace std;

Patient::Patient(NetworkClient *client) : networkClient(client), ownsNetworkClient(false)
{
    if (!networkClient)
    {
        networkClient = new NetworkClient();
        ownsNetworkClient = true;
    }
}

Patient::~Patient()
{
    if (ownsNetworkClient)
    {
        delete networkClient;
    }
}

bool Patient::signUp()
{
    string username, password;
    cout << "\nSign Up\n";
    cout << "Enter username: ";
    getline(cin, username);
    if (username.empty())
    {
        cout << "Username cannot be empty." << endl;
        return false;
    }

    cout << "Enter password: ";
    getline(cin, password);
    if (password.empty())
    {
        cout << "Password cannot be empty." << endl;
        return false;
    }

    const string key = "MEDICALSCHEDULER";
    return signUp(username, password, key);
}

bool Patient::signIn()
{
    string username, password;
    cout << "\nSign In\n";
    cout << "Enter username: ";
    getline(cin, username);
    if (username.empty())
    {
        cout << "Username cannot be empty." << endl;
        return false;
    }

    cout << "Enter password: ";
    getline(cin, password);
    if (password.empty())
    {
        cout << "Password cannot be empty." << endl;
        return false;
    }

    const string key = "MEDICALSCHEDULER";
    return signIn(username, password, key);
}

bool Patient::signUp(const string &username, const string &password, const string &key)
{
    string encryptedPassword = VigenereCipher::encrypt(password, key);
    bool success = networkClient->sendSignUpRequest(username, encryptedPassword);
    if (success)
    {
        this->username = username;
        this->password = encryptedPassword;
        cout << "Sign-up successful!" << endl;
    }
    else
    {
        cout << "Sign-up failed. The username might already be taken." << endl;
    }
    return success;
}

bool Patient::signIn(const string &username, const string &password, const string &key)
{
    string encryptedPassword = VigenereCipher::encrypt(password, key);
    bool success = networkClient->sendSignInRequest(username, encryptedPassword);
    if (success)
    {
        this->username = username;
        this->password = encryptedPassword;
        cout << "Sign-in successful!" << endl;
    }
    else
    {
        cout << "Sign-in failed. Please check your credentials." << endl;
    }
    return success;
}

void Patient::requestAppointment(const string &appointmentDetails)
{
    if (!isLoggedIn())
    {
        cout << "You must be logged in to request an appointment." << endl;
        return;
    }

    string doctorId, dateTime, details;
    cout << "\nRequest Appointment\n";
    cout << "Enter doctor name or ID: ";
    getline(cin, doctorId);
    cout << "Enter appointment date and time (YYYY-MM-DD HH:MM): ";
    getline(cin, dateTime);
    cout << "Please describe your symptoms or reason for visit: ";
    getline(cin, details);

    if (doctorId.empty() || dateTime.empty() || details.empty())
    {
        cout << "All fields are required." << endl;
        return;
    }

    string appointmentRequest = username + "|" + doctorId + "|" + dateTime + "|" + details;
    if (networkClient->sendAppointmentRequest(appointmentRequest))
    {
        cout << "Appointment request sent successfully!" << endl;
    }
    else
    {
        cout << "Failed to send appointment request. Please try again later." << endl;
    }
}