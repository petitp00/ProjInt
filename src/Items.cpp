#include "Items.h"

#include <iostream>

using namespace Item;

any Item::Banana;
any Item::Wood;
any Item::BananaPeel;

void Item::Init()
{
	Banana.name = "Banane";
	Banana.desc = "Mangeable. Plein de potassium. Miam. Laisse une pelure.";
	Banana.pos_in_texture_map = {0,0};
	Banana.edible = true; // !!!

	Wood.name = "Bois";
	Wood.desc = "Combustible. Utilisé pour faire d'autres objets.";
	Wood.pos_in_texture_map = {1, 0};
	Wood.edible = false;

	BananaPeel.name = "Pelure de banane";
	BananaPeel.desc = "Déchet bio. Compostable.";
	BananaPeel.pos_in_texture_map = {2, 0};
	BananaPeel.edible = false;
}

any Item::getItemByName(std::string name)
{
	if (name == "Banane") {
		return Item::Banana;
	}
	if (name == "Bois") {
		return Item::Wood;
	}
	if (name == "Pelure de banane") {
		return Item::BananaPeel;
	}

	std::cerr << "Item \"" << name << "\" does not exist" << std::endl;

	return any();
}
