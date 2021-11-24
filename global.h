#pragma once
using namespace std;

#define NO_STORAGE 0 //use this if you dont want to save files to your machine (e.g. certain host services), they will be saved to memory, which will probably result in a lot of lag
#define HYBRID_ONLY 0 //private is not fully working yet, xmpp and some other shit is broken, so its stuck loading lobby

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "lib/httplib.h"
using namespace httplib;

#include "lib/json.h"
using nlohmann::json;

httplib::Server server;

#define HOST "0.0.0.0" //localhost + open ips
#define PORT 80 //default http port

string workdir;
