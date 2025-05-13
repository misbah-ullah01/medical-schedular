#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

class User
{
public:
    User() : username(""), password("") {} // Added default constructor
    User(const string &username, const string &password);

    string getUsername() const;
    string getPassword() const;

    void setPassword(const string &password);

private:
    string username;
    string password; // This should be stored securely (e.g., hashed) in a real application
};

#endif // USER_H