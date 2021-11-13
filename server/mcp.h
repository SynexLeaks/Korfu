#pragma once
#include "../global.h"
#include "../tools.h"

#include <future>

map <string, json> profiles;
typedef pair <string, json> UniquePair;

namespace MCP {

	void saveconfig(string accountId, json configdata) {

#if NO_STORAGE
		profiles.find(accountId)->second = configdata;
#else
		ofstream print(workdir + "/config/" + accountId + ".json");
		print << configdata.dump();
#endif
	}

	json loadconfig(string accountId) {

#if NO_STORAGE
		return profiles.find(accountId)->second;
#else
		ifstream fconfigdata(workdir + "/config/" + accountId + ".json");
		string sconfigdata((std::istreambuf_iterator<char>(fconfigdata)),
			std::istreambuf_iterator<char>());
		return json::parse(sconfigdata);
#endif
	}

	json loadprofile(string profileId) {

		ifstream fprofiledata(workdir + "/profile/" + profileId + ".json");
		string sprofiledata((std::istreambuf_iterator<char>(fprofiledata)),
			std::istreambuf_iterator<char>());

		return json::parse(sprofiledata);
	}

	void initconfig(string accountId) {

#if NO_STORAGE
		if (profiles.find(accountId)->first != accountId)
			profiles.insert(UniquePair(accountId, loadprofile("config")));
#else 
		if (!filesystem::exists(workdir + "/config/" + accountId + ".json")) {

			filesystem::copy(workdir + "/profile/config.json", workdir + "/config/" + accountId + ".json");
		}
#endif
	}

	void update(void) {

		auto athena = []() {

			static string rawdata;
			httplib::Client client("https://fortnite-api.com"); 

			auto apipull = client.Get("/v2/cosmetics/br/", 
				[&](const char* data, size_t data_length) {
					rawdata.append(data, data_length);
					if (rawdata.back() != (char)"}")
						return true;
					return false;
				});

			static json athena = loadprofile("athena");
			json sandbox_loadout = loadprofile("athena")["items"].at("sandbox_loadout");
			json itemarray = json::parse(rawdata);
			
			for (int i = 0; i < itemarray.size(); i++) {

				string itemname = itemarray.at(i)["type"]["backendValue"] + ":" + itemarray.at(i)["id"];
				string itemvariants = itemarray.at(i)["variants"];

				athena["items"][itemname]["templateId"] = itemname;
				athena["items"][itemname]["attributes"]["max_level_bonus"] = 0;
				athena["items"][itemname]["attributes"]["level"] = 1;
				athena["items"][itemname]["attributes"]["item_seen"] = true;
				athena["items"][itemname]["attributes"]["rnd_sel_cnt"] = 0;
				athena["items"][itemname]["attributes"]["xp"] = 0;
				athena["items"][itemname]["attributes"]["favorite"] = false;

				athena["items"][itemname]["attributes"]["variants"].push_back(itemvariants);

				athena["items"][itemname]["quantity"] = 1;
			}

			ofstream print(workdir + "/profile/athena.json");
			print << athena.dump();

			log("Updated athena");
		};

		while (1) {

			athena();

			Sleep(432000000); //wait 5 days
		}
	}

	string response(json profilechanges, string profileId, int rvn) {

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
		try { //in case profile doesnt exist
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

					bool statusbool = statusboolarray.at(targetindex);
					string targetitem = targetitemarray.at(targetindex);

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
