#pragma once
#include "global.h"

#include "lib/termcolor.h"
#include <openssl/sha.h>

#include <filesystem>

#include <chrono>
#include <ctime>

#define log(tolog) \
std::cout << termcolor::cyan << "[Korfu]" << termcolor::yellow << "[" << tools::dateandtime() << "] " << termcolor::reset << tolog << std::endl;

#define error(errmsg) \
std::cout << termcolor::red << "[Error] " << termcolor::reset << errmsg << ": ";

namespace tools {

    void getworkdir() {

        char tworkdir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, tworkdir);

        workdir = tworkdir;
    }

    string throwerror(string errorMessage, int numericErrorCode, string originatingService, string messageVars, string intent = "prod-live", string errorCode = "errors.com.korfu.common") {

        json res = json::parse("{}");
        res["errorCode"] = errorCode;
        res["errorMessage"] = errorMessage;
        res["numericErrorCode"] = numericErrorCode;
        res["originatingService"] = originatingService;
        res["messageVars"] = messageVars;
        res["intent"] = intent;

        return res.dump();
    }

    string ISO8601date() {

        std::time_t ttime = std::time(nullptr);
        char result[120];

        std::strftime(result, sizeof(result), "%Y-%m-%dT%H:%M:%S.000Z", std::localtime(&ttime)); //milli-secs isnt supported, im not going to do a work-around lol

        return result;
    }

    string dateandtime() {

        std::time_t ttime = std::time(nullptr);
        char result[120];

        std::strftime(result, sizeof(result), "%Y-%m-%d %H:%M:%S", std::localtime(&ttime));

        return result;
    }

    std::string random_str(int length = 32) { //https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c

        std::string temp;
        static const char alphanum[] =
            "0123456789"
            "abcdefghijklmnopqrstuvwxyz";

        srand((unsigned)time(NULL) * getpid());

        temp.reserve(length);

        for (int i = 0; i < length; ++i)
            temp += alphanum[rand() % (sizeof(alphanum) - 1)];


        return temp;
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
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
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