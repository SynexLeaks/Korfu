#pragma once
#include "../global.h"
#include "../tools.h"

namespace MCP {

	void initconfig(string accountId) {

		if (!filesystem::exists(workdir + "/config/" + accountId + ".json")) {

			filesystem::copy(workdir + "/profile/config.json", workdir + "/config/" + accountId + ".json");
		}
	}

	void saveconfig(string accountId, json configdata) {

		ofstream print(workdir + "/config/" + accountId + ".json");
		print << configdata.dump();
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

	string response(json profilechanges, string profileId, int rvn) { //profileChangesBaseRevision

		json changesarray = json::parse("{}");
		changesarray["changeType"] = "fullProfileUpdate";
		changesarray["profile"] = profilechanges;

		json response = json::parse("{}");
		response["profileRevision"] = rvn +1;
		response["profileChangesBaseRevision"] = rvn;
		response["profileCommandRevision"] = rvn ? rvn - 0 + (1 - 0) : 1;
		response["serverTime"] = tools::ISO8601date();
		response["responseVersion"] = 1;

		response["profileId"] = profileId;
		response["profileChanges"].push_back(changesarray);

		return response.dump();
	}

	json profilechanges(string accountId, string profileId) {

		json profiledata;
		try {
			profiledata = loadprofile(profileId);
		}
		catch (...) { profiledata = json::parse("{}"); }
		json configdata = loadconfig(accountId);

		profiledata["_id"] = accountId;
		profiledata["accountId"] = accountId;
		profiledata["created"] = tools::ISO8601date();
		profiledata["updated"] = tools::ISO8601date();
		profiledata["profileId"] = profileId;

		if(profileId == "athena") {

			profiledata["stats"]["attributes"]["season_num"] = 18;
			profiledata["stats"]["attributes"]["level"] = configdata.at("level");
			profiledata["items"]["sandbox_loadout"]["attributes"]["banner_icon_template"] = configdata.at("banner_icon_template");
			profiledata["items"]["sandbox_loadout"]["attributes"]["banner_color_template"] = configdata.at("banner_color_template");
			profiledata["items"]["sandbox_loadout"]["attributes"]["locker_slots_data"]["slots"] = configdata.at("slots");

			for (int i = 0; i < configdata["favorites"].size(); i++) {

				string item = configdata["favorites"].at(i);
				profiledata["items"][item]["attributes"]["favorite"] = true;
			}
		}

		if (profileId == "common_core") {

			profiledata["items"]["Currency:MtxPurchased"]["quantity"] = configdata.at("vbucks");
		}

		return profiledata;
	}

	void Init() {

		server.Post("/fortnite/api/game/v2/profile/(.*)/client/(.*)", [](const auto& req, auto& res) {

			string accountId = req.matches[1];
			string command = req.matches[2];
			int rvn = stoi(req.get_param_value("rvn"));
			string profileId = req.get_param_value("profileId");

			initconfig(accountId);

			if (command == "SetCosmeticLockerSlot") {
				json jreqbody = json::parse(req.body);
				json configdata = loadconfig(accountId);

				string category = jreqbody.at("category");
				string targetitem = jreqbody.at("itemToSlot");

				if (category == "Dance") {
					int targetslot = jreqbody["slotIndex"];
					configdata["slots"][category]["items"].at(targetslot) = targetitem;
				}
				if (category == "ItemWrap") {
					int targetslot = jreqbody["slotIndex"].get<int>();
					if (targetslot > -1)
						configdata["slots"][category]["items"].at(targetslot) = targetitem;
					else if (targetslot == -1) {
						for (targetslot = 0; targetslot < 7; targetslot++)
							configdata["slots"][category]["items"].at(targetslot) = targetitem;
					}
				}
				else if (category != "ItemWrap" && category != "Dance") {
					configdata["slots"][category]["items"].at(0) = targetitem;
					configdata["slots"][category]["activeVariants"].at(0)["variants"] = jreqbody.at("variantUpdates");
				}

				saveconfig(accountId, configdata);
			}

			if (command == "SetCosmeticLockerBanner") {
				json jreqbody = json::parse(req.body);
				json configdata = loadconfig(accountId);

				if (jreqbody["bannerIconTemplateName"] != "None")
					configdata["banner_icon_template"] = jreqbody.at("bannerIconTemplateName");
				if (jreqbody["bannerColorTemplateName"] != "None")
					configdata["banner_color_template"] = jreqbody.at("bannerColorTemplateName");

				saveconfig(accountId, configdata);
			}

			if (command == "SetItemFavoriteStatusBatch") {
				json jreqbody = json::parse(req.body);
				json configdata = loadconfig(accountId);

				json statusboolarray = jreqbody["itemFavStatus"];
				json targetitemarray = jreqbody["itemIds"];

				for (int targetindex = 0; targetindex < targetitemarray.size(); targetindex++) {

					bool statusbool = statusboolarray.at(targetindex).get<bool>();
					string targetitem = targetitemarray.at(targetindex).get<string>();

					if (statusbool == 0) { //ok ill just loop through instead to get the index of it and erase then
						for (int favoriteindex = 0; favoriteindex < configdata["favorites"].size(); favoriteindex++) {
							if (configdata["favorites"].at(favoriteindex).get<string>() == targetitem) {
								configdata["favorites"].erase(favoriteindex);
							}
						}
					}
					else if (statusbool == 1 && !configdata["favorites"].contains(targetitem))
						configdata["favorites"].push_back(targetitem);
				}

				saveconfig(accountId, configdata);
			}

			res.set_content(response(profilechanges(accountId, profileId), profileId, rvn), "application/json"); //default profile

			if (command == "SetMtxPlatform") {
				json data = json::parse(R"([{"changeType": "statModified", "name": "current_mtx_platform", "value": ""}])");
				data.at(0)["value"] = req.body;
				res.set_content(response(data, profileId, rvn), "application/json");
			}
			});
	}
}
