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

	/*MCP::initconfig("e10c27c21d4c29558b6529f3ad4289e5");
	ofstream print(workdir + "/response.txt");
	print << MCP::response(MCP::profilechanges("e10c27c21d4c29558b6529f3ad4289e5", "common_core"), "common_core", 3);*/

	server.listen("0.0.0.0", 80);
}