#include "global.h"
#include <Windows.h>

#include "server/api.h"
#include "server/cloudstorage.h"
#include "server/content.h"
#include "server/mcp.h"

#include "server/other.h"

void main() {

	tools::getworkdir();

	API::Init();
	Cloudstorage::Init();
	MCP::Init();
	Content::Init();
	Other::Init();

	server.listen("0.0.0.0", 80);
}