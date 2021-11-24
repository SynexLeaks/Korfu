#pragma once
#include "../global.h"
#include "../tools.h"

#include <filesystem>

static map <string, string> uniques;
typedef pair <string, string> cUniquePair;

class Cloudstorage {

	static string unique(string filename) { //dynamicly assing a unique name

		if (uniques.find(filename) == uniques.end()) { //create unique if it does not exist

			string unqiuename = tools::random_str();

			while (unqiuename == uniques.find(unqiuename)->first)
				unqiuename = tools::random_str();

			uniques.insert(cUniquePair(filename, unqiuename));
			uniques.insert(cUniquePair(unqiuename, filename));

			return unqiuename;
		}

		return uniques.find(filename)->second;
	};

public:
	static void Init() {

		server.Get("/fortnite/api/cloudstorage/system", [](const auto& req, auto& res) {

			json sum = json::parse("[]");
			for (const auto& fileentry : filesystem::directory_iterator(workdir + "/cloudstorage/")) {

				const string filedata = tools::readFile(fileentry.path().string());

				json entry = json::parse("{}");

				entry["uniqueFilename"] = unique(fileentry.path().filename().string());
				entry["filename"] = fileentry.path().filename().string();
				entry["hash"] = tools::sha1(filedata);
				entry["hash256"] = tools::sha256(filedata);
				entry["length"] = filedata.length();
				entry["contentType"] = "application/octet-stream";
				entry["uploaded"] = tools::ISO8601date();
				entry["storageType"] = "S3";
				entry["doNotCache"] = true;

				sum.push_back(entry);
			}

			res.set_content(sum.dump(), "application/json");
			});

		server.Get("/fortnite/api/cloudstorage/system/(.*)", [](const auto& req, auto& res) {

			string responsedata;
			if (uniques.find(req.matches[1]) == uniques.end()) //if not found or summary wasnt used yet
				responsedata = ";not initialized";
			else
				string responsedata = tools::readFile(workdir + "/cloudstorage/" + uniques.find(req.matches[1])->second);

			res.set_content(responsedata, "application/octet-stream");
			});
	}
};
