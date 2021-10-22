#include "global.h"
#include <Windows.h>

#include "server/api/account.h"
#include "server/cloudstorage.h"
#include "server/content.h"
#include "server/mcp.h"

#include "server/other.h"

void main() {

	tools::getworkdir();
	filesystem::create_directory(workdir + "/config");
#ifdef _PRIV
	api::Init();
#endif
	Cloudstorage::Init();
	Content::Init();
	Other::Init();
	MCP::Init();

	server.set_error_handler([](const auto& req, auto& res) {

		error(res.status);
		switch (res.status) {
		case 500:
			res.set_content(tools::throwerror("Internal Server Error", res.status, "fortnite | backend", "Could not resolve " + req.path), "application/json");
			break;
		case 404:
			res.set_content(tools::throwerror("Not found", res.status, "fortnite | backend", "Route (" + req.path + ") not found"), "application/json");
			break;
		default:
			res.set_content(tools::throwerror("Unknown Error", res.status, "fortnite | backend", "UNKNOWN", "UNKNOWN"), "application/json"); //idfk
		}

		res.status = 204; //no need to add certain routes ig
		});

	server.set_logger([](const auto& req, auto& res) {

		if (!strstr(req.path.c_str(), "VerifyRealMoneyPurchase"))
			log(req.path);
		});

	log("Running @" << HOST << ":" << PORT);
	server.listen(HOST, PORT);
}