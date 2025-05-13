#pragma once
#include <string>
using namespace std;

class VigenereCipher
{
public:
    static string encrypt(const string &text, const string &key);
    static string decrypt(const string &text, const string &key);
};