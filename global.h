#pragma once
using namespace std;

#define NO_STORAGE 0
#define HYBRID_ONLY 0

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "lib/httplib.h"
using namespace httplib;

#include "lib/json.h"
using nlohmann::json;

httplib::Server server;

#define HOST "0.0.0.0" //localhost + open ips
#define PORT 80 //default http port

string workdir;
