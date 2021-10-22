#pragma once
#include "token.h"

namespace api {

	void Init() {

		server.Post("/account/api/oauth/token", [](const auto& req, auto& res) {

			json response = json::parse("{}");
			string granttype = json::parse(req.body)["grant_type"];
			
			res.set_content(response.dump(), "application/json");
			});
	}
}