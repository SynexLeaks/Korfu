#pragma once
#include "../global.h"

namespace Other {

	void Init() {

		server.Get("/lightswitch/api/service/bulk/status", [](const auto& req, auto& res) {

			res.set_content(json::parse(R"(
[{
	"serviceInstanceId": "fortnite",
	"status": "UP",
	"message": "Fortnite is online",
	"maintenanceUri": null,
	"overrideCatalogIds": [],
	"allowedActions": ["PLAY", "DOWNLOAD"],
	"banned": false,
	"launcherInfoDTO": {
		"appName": "Fortnite",
		"catalogItemId": "4fe75bbc5a674f4f9b356b5c90567da5",
		"namespace": "fn"
	}
}])").dump(), "application/json");
			});

		server.Get("/socialban/api/public/v1/(.*)", [](const auto& req, auto& res) {

			res.set_content(R"({"bans":[],"warnings":[]})", "application/json");
			});

		server.Get("/fortnite/api/v2/versioncheck/(.*)", [](const auto& req, auto& res) {

			res.set_content(R"({"type":"NO_UPDATE"})", "application/json");
			});
	}
}
