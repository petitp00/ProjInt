#include "Items.h"

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