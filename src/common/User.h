#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

// Represents a user (patient or admin)
class User
{
public:
    User() : username(""), password("") {}
    User(const string &username, const string &password);
    // Get username
    string getUsername() const;
    // Get password (should be hashed in production)
    string getPassword() const;
    // Set password
    void setPassword(const string &password);

private:
    string username;
    string password;
};

#endif // USER_H