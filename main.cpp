#include "global.h"
#include <Windows.h>

#include "server/api.h"
#include "server/cloudstorage.h"
#include "server/content.h"
#include "server/mcp.h"

#include "server/other.h"

void main() {

	tools::getworkdir();
	filesystem::create_directory(workdir + "/config");

	API::Init();
	Cloudstorage::Init();
	MCP::Init();
	Content::Init();
	Other::Init();

	server.set_error_handler([](const auto& req, auto& res) {

		error(res.status);
		switch (res.status) {
		case 500:
			res.set_content(tools::throwerror("Internal Server Error", res.status, "fortnite | backend", "Could not resolve " + req.path), "application/json");
			break;
		case 404:
			res.set_content(tools::throwerror("Not found", res.status, "fortnite | backend", "Route (" + req.path + ") does not exist"), "application/json");
			break;
		default:
			res.set_content(tools::throwerror("Unknown Error", res.status, "fortnite | backend", "UNKNOWN", "UNKNOWN"), "application/json"); //idfk
		}
		});

	server.set_logger([](const auto& req, auto& res) {

		log(req.path);
		});

	server.listen("0.0.0.0", 80);
}