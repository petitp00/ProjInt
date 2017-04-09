#include "Items.h"

#include <iostream>

using namespace Item;

any Item::Banana;
any Item::Wood;

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
}

any Item::getItemByName(std::string name)
{
	if (name == "Banane") {
		return Item::Banana;
	}
	if (name == "Bois") {
		return Item::Wood;
	}

	std::cerr << "Item \"" << name << "\" does not exist" << std::endl;

	return any();
}
