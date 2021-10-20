#pragma once
#include "../global.h"

namespace Content {

	void Init() {

		server.Get("/content/api/pages/fortnite-game/", [](const auto& req, auto& res) { //added mem modifiers to avoid crashes

			httplib::Client client("https://fortnitecontent-website-prod07.ol.epicgames.com");
			auto rawcontent = client.Get("/content/api/pages/fortnite-game/");
			static json content = json::parse(rawcontent->body);
            //CLEAN UP
            content["battleroyalenewsv2"]["news"].erase("motds");
            content["creativenewsv2"]["news"].erase("motds");
            content["emergencynoticev2"]["emergencynotices"].erase("emergencynotices");
            content.erase("comics");

			auto& lobbybg = content["dynamicbackgrounds"]["backgrounds"]["backgrounds"].at(0)["stage"];
			auto& shopbg = content["dynamicbackgrounds"]["backgrounds"]["backgrounds"].at(1)["stage"];
			auto& brnewsv2 = content["battleroyalenewsv2"]["news"];
			auto& crnewsv2 = content["creativenewsv2"]["news"];
			auto& emergencynoticev2 = content["emergencynoticev2"]["emergencynotices"];

			const json customenotice = json::parse(R"({
                            "hidden": false,
                            "_type": "CommonUI Emergency Notice Base",
                            "title": "Korfu",
                            "body": "Made by Pankiefield#9755"
                        })");

			const json customnews = json::parse(R"({
                        "entryType": "Website",
                        "image": "https://fire.api.pankiefield.live/cdn/fortnite-s18-banner.jpg",
                        "tileImage": "https://fire.api.pankiefield.live/cdn/fortnite-s18-banner.jpg",
                        "videoMute": false,
                        "hidden": false,
                        "tabTitleOverride": "Korfu",
                        "_type": "CommonUI Simple Message MOTD",
                        "title": "Korfu",
                        "body": "Made by Pankiefield#9755.",
                        "videoLoop": false,
                        "videoStreamingEnabled": false,
                        "sortingPriority": 0,
                        "id": "news1",
                        "videoAutoplay": false,
                        "videoFullscreen": false,
                        "spotlight": false,
                        "websiteURL": "https://google.com",
                        "websiteButtonText": "Open Google"
                    })");

			brnewsv2["motds"].push_back(customnews);
			crnewsv2["motds"].push_back(customnews);
			emergencynoticev2["emergencynotices"].push_back(customenotice);

			lobbybg = "season17";

			res.set_content(content.dump(), "application/json");
			});
	}
}
