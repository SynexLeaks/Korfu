#pragma once
#include "../global.h"
#include "../tools.h"

namespace MCP {

	void initconfig(string accountId) {

		if (!filesystem::exists(workdir + "/config/" + accountId + ".json")) {

			filesystem::copy(workdir + "/profile/config.json", workdir + "/config/" + accountId + ".json");
		}
	}

	void saveconfig(string accountId, string configdata) {

		ofstream print(workdir + "/config/" + accountId + ".json");
		print << configdata;
	}

	json loadconfig(string accountId) {

		ifstream fconfigdata(workdir + "/config/" + accountId + ".json");
		string sconfigdata((std::istreambuf_iterator<char>(fconfigdata)),
			std::istreambuf_iterator<char>());

		return json::parse(sconfigdata);
	}

	json loadprofile(string profileId) {

		ifstream fprofiledata(workdir + "/profile/" + profileId + ".json");
		string sprofiledata((std::istreambuf_iterator<char>(fprofiledata)),
			std::istreambuf_iterator<char>());

		return json::parse(sprofiledata);
	}


	string throwerror(string errorCode, string errorMessage, int numericErrorCode, string originatingService, string messageVars, string intent) {

		json res = json::parse("{}");
		res["errorCode"] = errorCode;
		res["errorMessage"] = errorMessage;
		res["numericErrorCode"] = numericErrorCode;
		res["originatingService"] = originatingService;
		res["messageVars"] = messageVars;
		res["intent"] = intent;

		return res;
	}

	string response(json profilechanges, string profileId, int rvn) {

		json changesarray = json::parse("{}");
		changesarray["changeType"] = "fullProfileUpdate";
		changesarray["profile"] = profilechanges;

		json response = json::parse("{}");
		response["profileRevision"] = rvn + 1;
		response["profileChangesBaseRevision"] = rvn;
		response["profileCommandRevision"] = rvn;
		response["serverTime"] = tools::ISO8601Date();
		response["responseVersion"] = 1;

		response["profileId"] = profileId;
		response["profileChanges"].push_back(changesarray);

		return response;
	}

	json profilechanges(string accountId, string profileId) {

		json profiledata = loadprofile(accountId);
		json configdata = loadconfig(profileId);

		profiledata["accountId"] = accountId;
		profiledata["created"] = tools::ISO8601Date();
		profiledata["updated"] = tools::ISO8601Date();
		profiledata["profileId"] = profileId;

		if(profileId == "athena") {

			profiledata["stats"]["attributes"]["season_num"] = 18;
			profiledata["stats"]["attributes"]["level"] = configdata["level"];
			profiledata["items"]["sandbox_loadout"]["attributes"]["banner_icon_template"] = configdata["banner_icon_template"];
			profiledata["items"]["sandbox_loadout"]["attributes"]["banner_color_template"] = configdata["banner_color_template"];
			profiledata["items"]["sandbox_loadout"]["attributes"]["locker_slots_data"]["slots"] = configdata["slots"];

			for (int i = 0; i < configdata["favorites"].size(); i++) {

				string name = configdata["favorites"].at(i);
				profiledata["items"][name]["attributes"]["favorite"] = true;
			}
		}

		if (profileId == "common_core") {

			profiledata["items"]["Currency:MtxPurchased"]["quantity"] = configdata["vbucks"];
		}

		return profiledata.dump();
	}

	void Init() {

		server.Post("/fortnite/api/game/v2/profile/(.*)/client/(.*)", [](const Request& req, Response& res) {

			string account = req.matches[1];
			string command = req.matches[2];

			InitConfig(account);

			string profile = req.get_param_value("profileId");
			int rvn = stoi(req.get_param_value("rvn"));

			json profiledata = json::parse(LoadProfile(profile));
			json config = json::parse(LoadConfig(account));
			json bodydata = json::parse(req.body.c_str());

			if (!strstr(command.c_str(), "VerifyRealMoneyPurchase"))
				log("MCP: " + account + ", " + profile + ": " + command);

			switch (str2int(command.c_str())) {
			case str2int("ClientQuestLogin"):
			case str2int("QueryProfile"):
				switch (str2int(profile.c_str())) {
				case str2int("collections"):
					res.set_content(createResponse(createUsual(account, profiledata), profile, rvn), "application/json");
					break;
				case str2int("athena"):
				case str2int("profile0"):
					res.set_content(createResponse(createAthena(account, profiledata, config), profile, rvn), "application/json");
					break;
				case str2int("creative"):
					res.set_content(createResponse(createCreative(account, profiledata), profile, rvn), "application/json");
					break;
				case str2int("common_core"):
					res.set_content(createResponse(createCommonCore(account, profiledata, config), profile, rvn), "application/json");
					break;
				case str2int("common_public"):
					res.set_content(createResponse(createUsual(account, profiledata), profile, rvn), "application/json");
					break;
				}
				break;
			case str2int("SetMtxPlatform"):
				res.set_content(createResponse("[{ \"changeType\":  \"statModified\", \"name\" : \"current_mtx_platform\", \"value\" : " + req.body + "}]", profile, rvn), "application/json");
				break;
			case str2int("VerifyRealMoneyPurchase"):
				res.set_content(createResponse(createCommonCore(account, profiledata, config), profile, rvn), "application/json");
				break;

			case str2int("SetCosmeticLockerBanner"):
				if (bodydata["bannerIconTemplateName"] != "None")
					config["BannerIconTemplate"] = bodydata.at("bannerIconTemplateName");
				if (bodydata["bannerColorTemplateName"] != "None")
					config["BannerColorTemplate"] = bodydata.at("bannerColorTemplateName");

				SaveConfig(account, config.dump());
				res.set_content(createResponse(createAthena(account, profiledata, config), profile, rvn), "application/json");
				break;

			case str2int("SetItemFavoriteStatusBatch"): {

				json favorize = bodydata["itemFavStatus"];
				json targetarray = bodydata["itemIds"];
				int index = 0;

				for (auto& unused : targetarray) {

					string fbool = favorize.at(index).dump();
					json target = targetarray.at(index);

					if (fbool == "true") {
						config["favorites"].push_back(target);
					}
					if (fbool == "false") {
						config["favorites"].erase(config["favorites"].find(target));
					}

					index = index + 1;
				}
				rvn = rvn + 1;
				SaveConfig(account, config.dump());
				res.set_content(createResponse(createAthena(account, profiledata, config), profile, rvn), "application/json");
				break;
			}

			case str2int("SetCosmeticLockerSlot"):
				string category = bodydata.at("category");
				string target = bodydata.at("itemToSlot");
				int slot = bodydata["slotIndex"].get<int>();

				if (category != "ItemWrap" && category != "Dance") {
					config["slots"][category].erase("items");
					config["slots"][category]["items"].push_back(target);
					config["slots"][category]["activeVariants"] = json::parse("[{\"variants\": " + bodydata.at("variantUpdates").dump() + "}]");
				}

				if (category == "Dance") {

					config["slots"][category]["items"].at(slot) = target;
				}

				else if (category == "ItemWrap") {

					if (slot > -1) {
						config["slots"][category]["items"].at(slot) = target;
						config["slots"][category]["activeVariants"].at(slot) = json::parse("[{\"variants\": " + bodydata.at("variantUpdates").dump() + "}]");
					}

					else {
						for (slot = 0; slot < 7; slot++) {
							config["slots"][category]["items"].at(slot) = target;
							config["slots"][category]["activeVariants"].at(slot) = json::parse("[{\"variants\": " + bodydata.at("variantUpdates").dump() + "}]");
						}
					}
				}

				SaveConfig(account, config.dump());
				res.set_content(createResponse(createAthena(account, profiledata, config), "athena", rvn), "application/json");

				break;
			}
			});

			server.Post("/fortnite/api/game/v2/profile/(.*)/client/(.*)", [](const auto& req, auto& res) {

				string accountId = req.matches[1];
				string command = req.matches[2];
				int rvn = stoi(req.get_param_value("rvn"));
				string profileId = req.get_param_value("profileId");

				initconfig(accountId););

				if (command == "ClientQuestLogin" || "ClientQuestLogin") {

					string profileId = req.get_param_value("profileId");
					if (profileId == "collections")
						res.set_content(response(profilechanges(accountId, profileId), profileId, rvn), "application/json");
				}

				switch (str2int(command.c_str())) {
				case str2int("ClientQuestLogin"):
				case str2int("QueryProfile"):
					switch (str2int(profile.c_str())) {
					case str2int("collections"):
						
						break;
					case str2int("athena"):
					case str2int("profile0"):
						res.set_content(createResponse(createAthena(account, profiledata, config), profile, rvn), "application/json");
						break;
					case str2int("creative"):
						res.set_content(createResponse(createCreative(account, profiledata), profile, rvn), "application/json");
						break;
					case str2int("common_core"):
						res.set_content(createResponse(createCommonCore(account, profiledata, config), profile, rvn), "application/json");
						break;
					case str2int("common_public"):
						res.set_content(createResponse(createUsual(account, profiledata), profile, rvn), "application/json");
						break;
					}
					break;
				case str2int("SetMtxPlatform"):
					res.set_content(createResponse("[{ \"changeType\":  \"statModified\", \"name\" : \"current_mtx_platform\", \"value\" : " + req.body + "}]", profile, rvn), "application/json");
					break;
				case str2int("VerifyRealMoneyPurchase"):
					res.set_content(createResponse(createCommonCore(account, profiledata, config), profile, rvn), "application/json");
					break;

				case str2int("SetCosmeticLockerBanner"):
					if (bodydata["bannerIconTemplateName"] != "None")
						config["BannerIconTemplate"] = bodydata.at("bannerIconTemplateName");
					if (bodydata["bannerColorTemplateName"] != "None")
						config["BannerColorTemplate"] = bodydata.at("bannerColorTemplateName");

					SaveConfig(account, config.dump());
					res.set_content(createResponse(createAthena(account, profiledata, config), profile, rvn), "application/json");
					break;

				case str2int("SetItemFavoriteStatusBatch"): {

					json favorize = bodydata["itemFavStatus"];
					json targetarray = bodydata["itemIds"];
					int index = 0;

					for (auto& unused : targetarray) {

						string fbool = favorize.at(index).dump();
						json target = targetarray.at(index);

						if (fbool == "true") {
							config["favorites"].push_back(target);
						}
						if (fbool == "false") {
							config["favorites"].erase(config["favorites"].find(target));
						}

						index = index + 1;
					}
					rvn = rvn + 1;
					SaveConfig(account, config.dump());
					res.set_content(createResponse(createAthena(account, profiledata, config), profile, rvn), "application/json");
					break;
				}

				case str2int("SetCosmeticLockerSlot"):
					string category = bodydata.at("category");
					string target = bodydata.at("itemToSlot");
					int slot = bodydata["slotIndex"].get<int>();

					if (category != "ItemWrap" && category != "Dance") {
						config["slots"][category].erase("items");
						config["slots"][category]["items"].push_back(target);
						config["slots"][category]["activeVariants"] = json::parse("[{\"variants\": " + bodydata.at("variantUpdates").dump() + "}]");
					}

					if (category == "Dance") {

						config["slots"][category]["items"].at(slot) = target;
					}

					else if (category == "ItemWrap") {

						if (slot > -1) {
							config["slots"][category]["items"].at(slot) = target;
							config["slots"][category]["activeVariants"].at(slot) = json::parse("[{\"variants\": " + bodydata.at("variantUpdates").dump() + "}]");
						}

						else {
							for (slot = 0; slot < 7; slot++) {
								config["slots"][category]["items"].at(slot) = target;
								config["slots"][category]["activeVariants"].at(slot) = json::parse("[{\"variants\": " + bodydata.at("variantUpdates").dump() + "}]");
							}
						}
					}

					SaveConfig(account, config.dump());
					res.set_content(createResponse(createAthena(account, profiledata, config), "athena", rvn), "application/json");

					break;
				}
				});
	}
}