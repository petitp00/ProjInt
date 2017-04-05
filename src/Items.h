#pragma once

#include <string>

#include <SFML/System/Vector2.hpp>

namespace Item
{
	static const std::string texture_map_file = "Items.png";
	static const float items_texture_size = 16.f;

	struct any
	{
		any() : edible(false) {}
		std::string name;
		std::string desc;

		sf::Vector2i pos_in_texture_map;

		bool edible;
	};

	extern any Banana;
	extern any Wood;

	void Init();
	any getItemByName(std::string name);
}
