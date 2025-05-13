#ifndef VIGENERECIPHER_H
#define VIGENERECIPHER_H

#include <string>
using namespace std;

class VigenereCipher
{
public:
    // Constructor to initialize the key
    VigenereCipher();
    VigenereCipher(const string &key);
    string encrypt(const string &plaintext);
    string decrypt(const string &ciphertext);

    static string encrypt(const string &plaintext, const string &key);
    static string decrypt(const string &ciphertext, const string &key);

private:
    string key;
    string generateKey(const string &text);
};

#endif // VIGENERECIPHER_H