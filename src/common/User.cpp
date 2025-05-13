#include "User.h"
#include <string>
using namespace std;

User::User(const std::string &username, const string &password)
    : username(username), password(password) {}

string User::getUsername() const
{
    return username;
}

string User::getPassword() const
{
    return password;
}

void User::setPassword(const string &newPassword)
{
    password = newPassword;
}