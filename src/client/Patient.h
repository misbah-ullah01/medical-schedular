#ifndef PATIENT_H
#define PATIENT_H

#include <string>
#include "NetworkClient.h"
using namespace std;

class Patient
{
public:
    Patient(NetworkClient *client = nullptr);
    ~Patient();

    bool signUp();
    bool signIn();
    bool signUp(const string &username, const string &password, const string &key);
    bool signIn(const string &username, const string &password, const string &key);
    void requestAppointment(const string &appointmentDetails);

    string getUsername() const { return username; }
    bool isLoggedIn() const { return !username.empty(); }

private:
    string username;
    string password;
    NetworkClient *networkClient;
    bool ownsNetworkClient;
    string encryptPassword(const string &password);
};

#endif // PATIENT_H