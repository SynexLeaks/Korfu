#pragma once
#include "../global.h"

namespace Content {

	void Init() {

		server.Get("/content/api/pages/fortnite-game/", [](const auto& req, auto& res) {

			httplib::Client client("https://fortnitecontent-website-prod07.ol.epicgames.com");
			auto rawcontent = client.Get("/content/api/pages/fortnite-game/");

			/* 
			static json content = json::parse(rawcontent->body); //can be used to change stuff like news, bg, emergency warning
			content["dynamicbackgrounds"]["backgrounds"]["backgrounds"].at(0)["stage"] = "season17";
			res.set_content(content.dump(), "application/json");
			*/

			res.set_content(rawcontent->body, "application/json");
			});
	}
}
