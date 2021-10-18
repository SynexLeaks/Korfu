#pragma once
#include "global.h"
#include <openssl/sha.h>

#include <filesystem>

namespace tools {

    void getworkdir() {

        char tworkdir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, tworkdir);

        workdir = tworkdir;
    }

    string sha256(const string str)
    {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, str.c_str(), str.size());
        SHA256_Final(hash, &sha256);
        stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            ss << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    string sha1(const string str)
    {
        unsigned char hash[SHA_DIGEST_LENGTH];
        SHA_CTX sha;
        SHA1_Init(&sha);
        SHA1_Update(&sha, str.c_str(), str.size());
        SHA1_Final(hash, &sha);
        stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        {
            ss << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    string readFile(string path) {

        std::ifstream filestream(path);
        std::string filedata((std::istreambuf_iterator<char>(filestream)),
            std::istreambuf_iterator<char>());

        return filedata;
    }
}