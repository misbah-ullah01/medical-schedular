// Implements User class methods
#include "User.h"
#include <string>
using namespace std;

// Initialize user with username and password
User::User(const string &username, const string &password)
    : username(username), password(password) {}

// Get username
string User::getUsername() const
{
    return username;
}

// Get password
string User::getPassword() const
{
    return password;
}

// Set password
void User::setPassword(const string &newPassword)
{
    password = newPassword;
}