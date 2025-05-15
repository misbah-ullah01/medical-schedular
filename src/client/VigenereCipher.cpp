#include "VigenereCipher.h"
#include <string>
#include <cctype>

using namespace std;

string VigenereCipher::encrypt(const string &plaintext, const string &key)
{
    string ciphertext;
    int keyIndex = 0;
    for (char c : plaintext)
    {
        if (isalpha(c))
        {
            char base = isupper(c) ? 'A' : 'a';
            ciphertext += ((c - base + (toupper(key[keyIndex % key.length()]) - 'A')) % 26 + base);
            keyIndex++;
        }
        else
        {
            ciphertext += c;
        }
    }
    return ciphertext;
}

string VigenereCipher::decrypt(const string &ciphertext, const string &key)
{
    string plaintext;
    int keyIndex = 0;
    for (char c : ciphertext)
    {
        if (isalpha(c))
        {
            char base = isupper(c) ? 'A' : 'a';
            plaintext += ((c - base - (toupper(key[keyIndex % key.length()]) - 'A') + 26) % 26 + base);
            keyIndex++;
        }
        else
        {
            plaintext += c;
        }
    }
    return plaintext;
}