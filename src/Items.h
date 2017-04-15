#pragma once

#include <string>
#include <map>
#include "Globals.h"

#include <SFML/System/Vector2.hpp>

namespace Item
{
	static const std::string texture_map_file = "Items.png";
	static const float items_texture_size = 16.f;

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

		bool operator==(any& b) {
			return this->name == b.name;
		}
	};

	struct Food : public any
	{
		int spoil_level = 0; // max 100
		ItemType junk_created = none;
	};

	struct BioJunk : public any
	{
		BioJunk() : any() { desc = "Déchet compostable"; }
		int compost_time = 100; // decreases to 0, then disappears
	};

	struct Tool : public any
	{
		int durability = 0; // max 100, + x per use
	};
	struct Bowl : public any
	{
		void UpdatePosInTextureMap();
		int water_level = 0; // 4: filled, 0: empty
	};

	ItemType getItemTypeByName(const std::string& name);

	static bool IsFood(ItemType type) {
		return (type == banana || type == apple || type == carrot);
	}

	static bool IsBioJunk(ItemType type) {
		return (type == banana_peel || type == apple_core || type == carrot_top);
	}

	static bool IsTool(ItemType type) {
		return (type == axe || type == hoe);
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

		static int CreateItem(ItemType type); // returns id
		static void DeleteItem(int id);

		static any*		getAny		(int id) { return items[id]; }
		static Food*	getFood		(int id) { return foods[id]; }
		static BioJunk* getBioJunk	(int id) { return bio_junks[id]; }
		static Tool*	getTool		(int id) { return tools[id]; }
		static Bowl*	getBowl		(int id) { return bowls[id]; }
	};
}
