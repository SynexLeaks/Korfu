#pragma once
#include "../global.h"
#include "../tools.h"

#include <filesystem>

namespace Cloudstorage {

	map <string, string> uniques;

	void Init() {

		typedef pair <string, string> UniquePair;
		uniques.insert(UniquePair("DefaultGame.ini", "4050c8b941ab4389841792dd5d62b9c9"));
		uniques.insert(UniquePair("DefaultEngine.ini", "f3066a8eb12946ddbe7f081d629f61b1"));
		uniques.insert(UniquePair("DefaultRuntimeOptions.ini", "6400cbfae00240bbaa58a394fbccfe69"));

		uniques.insert(UniquePair("4050c8b941ab4389841792dd5d62b9c9", "DefaultGame.ini"));
		uniques.insert(UniquePair("f3066a8eb12946ddbe7f081d629f61b1", "DefaultEngine.ini"));
		uniques.insert(UniquePair("6400cbfae00240bbaa58a394fbccfe69", "DefaultRuntimeOptions.ini"));


		server.Get("/fortnite/api/cloudstorage/system", [](const auto& req, auto& res) {

			json conclusion = json::parse("[]");

			for (const auto& fileentry : filesystem::directory_iterator(workdir + "/cloudstorage/")) {

				string filedata = tools::readFile(fileentry.path().string());

				json entry = json::parse("{}");

				entry["uniqueFilename"] = uniques.find(fileentry.path().filename().string())->second;
				entry["filename"] = fileentry.path().filename();
				entry["hash"] = tools::sha1(filedata);
				entry["hash256"] = tools::sha256(filedata);
				entry["length"] = filedata.length();
				entry["contentType"] = "application/octet-stream";
				entry["storageType"] = "S3";
				entry["doNotCache"] = false;

				conclusion.push_back(entry);
			}

			res.set_content(conclusion.dump(), "application/json");
			});

		server.Get("/fortnite/api/cloudstorage/system/(.*)", [](const auto& req, auto& res) {

			if (req.matches[1] != "config") {
				string filename = uniques.find(req.matches[1])->second;
				res.set_content(tools::readFile(workdir + "/cloudstorage/" + filename), "application/octet-stream");
			}

			if (req.matches[1] == "config") {
				res.set_content(json::parse(R"(
{
	"lastUpdated": "2021-10-14T19:25:03.908Z",
	"disableV2": false,
	"isAuthenticated": false,
	"enumerateFilesPath": "/api/cloudstorage/system",
	"transports": {
		"McpProxyTransport": {
			"name": "McpProxyTransport",
			"type": "ProxyStreamingFile",
			"appName": "fortnite",
			"isEnabled": false,
			"isRequired": true,
			"isPrimary": true,
			"timeoutSeconds": 30,
			"priority": 10
		},
		"McpSignatoryTransport": {
			"name": "McpSignatoryTransport",
			"type": "ProxySignatory",
			"appName": "fortnite",
			"isEnabled": false,
			"isRequired": false,
			"isPrimary": false,
			"timeoutSeconds": 30,
			"priority": 20
		}
	}
})").dump(), "application/json"); //parsing to json to format
			}

			res.set_content("", "application/octet-stream");
			
			});
	}
}