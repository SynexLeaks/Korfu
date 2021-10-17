#include "global.h"
#include <Windows.h>

void main() {

	server.Get("/", [](const httplib::Request&, httplib::Response& res) {
		res.set_content("Hello World!", "text/plain");
		});

	server.listen("0.0.0.0", 80);
}