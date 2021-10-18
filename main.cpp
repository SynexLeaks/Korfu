#include "global.h"
#include <Windows.h>

#include "server/cloudstorage.h"

void main() {

	tools::getworkdir();

	Cloudstorage::Init();

	server.listen("0.0.0.0", 80);
}