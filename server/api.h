#pragma once
#include "../global.h"

namespace API {

	void Init() {

		server.Get("/fortnite/api/v2/versioncheck/(.*)", [](const auto& req, auto& res) {

			res.set_content(R"({"type":"NO_UPDATE"})", "application/json");
			});
	}
}