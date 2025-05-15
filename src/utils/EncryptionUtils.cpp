// Implements Vigenère cipher encryption/decryption
#include "EncryptionUtils.h"
#include <string>
#include <vector>

using namespace std;

// Encrypts text using Vigenère cipher
string encrypt(const string &text, const string &key)
{
    string result;
    int keyLength = key.length();
    for (size_t i = 0; i < text.length(); ++i)
    {
        char encryptedChar = (text[i] + key[i % keyLength]) % 256;
        result += encryptedChar;
    }
    return result;
}

// Decrypts text using Vigenère cipher
string decrypt(const string &encryptedText, const string &key)
{
    string result;
    int keyLength = key.length();
    for (size_t i = 0; i < encryptedText.length(); ++i)
    {
        char decryptedChar = (encryptedText[i] - key[i % keyLength] + 256) % 256;
        result += decryptedChar;
    }
    return result;
}