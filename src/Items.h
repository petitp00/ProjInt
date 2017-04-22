#pragma once

#include <string>
#include <map>
#include <vector>

#include "Globals.h"

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>

namespace Item
{
	static const std::string texture_map_file = "Textures_Icones.png";
	static const float items_texture_size = 32.f;

	enum ItemType {
		none = -1,
		banana = 0,
		wood = 1,
		banana_peel = 2,
		axe = 3,
		hoe = 4,
		bowl = 5,
		apple = 6,
		apple_core = 7,
		carrot = 8,
		carrot_top = 9
	};

	struct any
	{
		std::string name;
		std::string desc;
		vec2i pos_in_texture_map;

		virtual std::vector<std::string> getSaveData() {
			std::cerr << "Using Item::any::getSaveData(), probably an error" << std::endl;
			return {};
		}

		bool operator==(any& b) {
			return this->name == b.name;
		}
	};

	struct Food : public any
	{
		int spoil_level = 0; // max 100
		ItemType junk_created = none;

		std::vector<std::string> getSaveData() override {
			return {
				std::to_string(spoil_level)
			};
		}
	};

	struct BioJunk : public any
	{
		BioJunk() : any() { desc = "Déchet compostable"; }
		//bool in_compost; ???
		int compost_time = 100; // decreases to 0, then disappears

		std::vector<std::string> getSaveData() override {
			return {
				std::to_string(compost_time)
			};
		}
	};

	struct Tool : public any
	{
		int durability = 0; // max 100, + x per use
		sf::Time use_speed = sf::seconds(0.7f);

		std::vector<std::string> getSaveData() override {
			return {
				std::to_string(durability)
			};
		}
	};
	struct Bowl : public Tool
	{
		void UpdatePosInTextureMap();
		int water_level = 0; // 4: filled, 0: empty

		std::vector<std::string> getSaveData() override {
			return {
				std::to_string(water_level)
			};
		}
	};

	ItemType getItemTypeByName(const std::string& name);
	sf::IntRect getItemTextureRect(ItemType type);

	static bool IsFood(ItemType type) {
		return (type == banana || type == apple || type == carrot);
	}

	static bool IsBioJunk(ItemType type) {
		return (type == banana_peel || type == apple_core || type == carrot_top);
	}

	static bool IsTool(ItemType type) {
		return (type == axe || type == hoe || type == bowl);
	}

	static bool IsBowl(ItemType type) {
		return (type == bowl);
	}

	class Manager
	{
	public:
		static int last_id;
		static std::map<int, any*> items;
		static std::map<int, Food*> foods;
		static std::map<int, BioJunk*> bio_junks;
		static std::map<int, Tool*> tools;
		static std::map<int, Bowl*> bowls;

		static int CreateItem(ItemType type, std::vector<std::string> save_data = std::vector<std::string>()); // returns id
		static void DeleteItem(int id);

		static ItemType getItemType(int id);
		static any*		getAny		(int id) { return items[id]; }
		static Food*	getFood		(int id) { return foods[id]; }
		static BioJunk* getBioJunk	(int id) { return bio_junks[id]; }
		static Tool*	getTool		(int id) { return tools[id]; }
		static Bowl*	getBowl		(int id) { return bowls[id]; }
	};
}
