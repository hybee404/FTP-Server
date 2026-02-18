#include "checksum.hpp"
#include <openssl/evp.h>
#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <sstream>


std::string Checksum::calculate_sha256(const std::string& filepath, bool show_progress) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate); // to open file to be hashed
    if (!file.is_open()) return "";

    std::streamsize total_size = file.tellg(); //get the current position of pointer reading the file
    file.seekg(0, std::ios::beg); // Move back to start for reading

    uint64_t bytes_processed = 0;

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new(); //an API for managing operation like hashing
    if (!mdctx) return "";

    if (!EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr)) {
        EVP_MD_CTX_free(mdctx);
        return "";
    }

    std::vector<char> buffer(65536); // 64KB buffer

    while (file.read(buffer.data(), buffer.size()) || file.gcount() > 0) {
        std::streamsize bytes = file.gcount();
        EVP_DigestUpdate(mdctx, buffer.data(), bytes); //filling buffer into digest

        if (show_progress && total_size > 0) {
            bytes_processed += bytes;
            int progress = static_cast<int>((static_cast<double>(bytes_processed) / total_size) * 100);
        }
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;
    EVP_DigestFinal_ex(mdctx, hash, &lengthOfHash);
    EVP_MD_CTX_free(mdctx);

    if (show_progress) std::cout << std::endl;

    std::stringstream ss;
    for (unsigned int i = 0; i < lengthOfHash; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

