#pragma once
#include "../global.h"
#include "../tools.h"

#include <filesystem>

namespace Cloudstorage {

	map <string, string> uniques;
	typedef pair <string, string> UniquePair;

	string unique(string filename) {

		if (uniques.find(filename) == uniques.end()) { //create unique if it does not exist

			string unqiuename = tools::random_str();

			while(unqiuename == uniques.find(unqiuename)->first)
				unqiuename = tools::random_str();

			uniques.insert(UniquePair(filename, unqiuename));
			uniques.insert(UniquePair(unqiuename, filename));

			return unqiuename;
		}

		return uniques.find(filename)->second;
	};

	void Init() {

		server.Get("/fortnite/api/cloudstorage/system", [](const auto& req, auto& res) {

			json conclusion = json::parse("[]");
			for (const auto& fileentry : filesystem::directory_iterator(workdir + "/cloudstorage/")) {

				string filedata = tools::readFile(fileentry.path().string());

				json entry = json::parse("{}");

				entry["uniqueFilename"] = unique(fileentry.path().filename().string());
				entry["filename"] = fileentry.path().filename().string();
				entry["hash"] = tools::sha1(filedata);
				entry["hash256"] = tools::sha256(filedata);
				entry["length"] = filedata.length();
				entry["contentType"] = "application/octet-stream";
				entry["uploaded"] = tools::ISO8601Date();
				entry["storageType"] = "S3";
				entry["doNotCache"] = false;

				conclusion.push_back(entry);
			}

			res.set_content(conclusion.dump(), "application/json");
			});

		server.Get("/fortnite/api/cloudstorage/system/(.*)", [](const auto& req, auto& res) {

			res.set_content(tools::readFile(workdir + "/cloudstorage/" + uniques.find(req.matches[1])->second), "application/octet-stream");
			});
	}
} 

/*if (req.matches[1] == "config") {
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
})").dump(), "application/json"); //parsing to json to format*/