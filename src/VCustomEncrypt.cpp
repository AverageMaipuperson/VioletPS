#include "../vendor/robtop/VCustomEncrypt.hpp"
#include <iostream>
#include <ctime>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <iomanip>

std::string VCustomEncrypt::ce_encode(std::string input) {
     std::stringstream hex_encoded;
    for (unsigned char c : input) {
        hex_encoded << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
    std::string hex_str = hex_encoded.str();

    std::string shifted_hex_result;
    for (char c : hex_str) {
        shifted_hex_result += static_cast<char>(c + 0x10);
    }
    return shifted_hex_result;
}

std::string VCustomEncrypt::ce_decode(const std::string& shifted_hex_input) {
      std::string unshifted_hex;
    for (char c : shifted_hex_input) {
        unshifted_hex += static_cast<char>(c - 0x10);
    }

    if (unshifted_hex.length() % 2 != 0) {
        throw std::invalid_argument("invalid length");
    }

    std::string result;
    result.reserve(unshifted_hex.length() / 2); 

    for (size_t i = 0; i < unshifted_hex.length(); i += 2) {
        std::string byteString = unshifted_hex.substr(i, 2);
        char byte = static_cast<char>(std::stoul(byteString, nullptr, 16));
        result.push_back(byte);
    }
    return result;
}