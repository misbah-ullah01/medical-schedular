#include "EncryptionUtils.h"
#include <string>
#include <vector>

std::string encrypt(const std::string& text, const std::string& key) {
    std::string result;
    int keyLength = key.length();
    for (size_t i = 0; i < text.length(); ++i) {
        char encryptedChar = (text[i] + key[i % keyLength]) % 256;
        result += encryptedChar;
    }
    return result;
}

std::string decrypt(const std::string& encryptedText, const std::string& key) {
    std::string result;
    int keyLength = key.length();
    for (size_t i = 0; i < encryptedText.length(); ++i) {
        char decryptedChar = (encryptedText[i] - key[i % keyLength] + 256) % 256;
        result += decryptedChar;
    }
    return result;
}