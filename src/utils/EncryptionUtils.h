#pragma once
#include <string>
using namespace std;

// Provides static methods for Vigenère cipher encryption/decryption
class VigenereCipher
{
public:
    // Encrypt text with key
    static string encrypt(const string &text, const string &key);
    // Decrypt text with key
    static string decrypt(const string &text, const string &key);
};