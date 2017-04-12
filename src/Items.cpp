#include "Items.h"

#include <iostream>

using namespace Item;
using namespace std;

void Bowl::UpdatePosInTextureMap()
{
	if (water_level == 0) pos_in_texture_map = {5,0};
	else if (water_level == 1) pos_in_texture_map = {6,0};
	else if (water_level == 2) pos_in_texture_map = {7,0};
	else if (water_level == 3) pos_in_texture_map = {0,1};
	else if (water_level == 4) pos_in_texture_map = {1,1};
}

int Manager::last_id = 0;
map<int, any*>		Manager::items;
map<int, Food*>		Manager::foods;
map<int, BioJunk*>	Manager::bio_junks;
map<int, Tool*>		Manager::tools;
map<int, Bowl*>		Manager::bowls;

Food* make_food(ItemType type) {
	Food* f = new Food;
	if (type == banana) {
		f->name = "Banane";
		f->desc = "Fruit. Mangeable. Laisse une pelure.";
		f->pos_in_texture_map = {0,0};
		f->junk_created = banana_peel;
		return f;
	}
	if (type == apple) {
		f->name = "Pomme";
		f->desc = "Fruit. Mangeable. Laisse un coeur.";
		f->pos_in_texture_map = {2,1};
		f->junk_created = apple_core;
		return f;
	}
	if (type == carrot) {
		f->name == "Carotte";
		f->desc = "Légume. Mangeable. Laisse un bout et des graines.";
		f->pos_in_texture_map = {4,1};
		f->junk_created = carrot_top;
	}

	f->name == "Item was not a Food";
	return f;
}

BioJunk* make_bio_junk(ItemType type) {
	BioJunk* bj = new BioJunk;
	if (type == banana_peel) {
		bj->name = "Pelure de banane";
		bj->pos_in_texture_map = {2, 0};
		return bj;
	}
	if (type == apple_core) {
		bj->name = "Coeur de pomme";
		bj->pos_in_texture_map = {3, 1};
		return bj;
	}
	if (type == carrot_top) {
		bj->name = "Bout de carotte";
		bj->pos_in_texture_map = {5, 1};
		return bj;
	}

	bj->name = "Item was not a BioJunk";
	return bj;
}

Tool* make_tool(ItemType type) {
	Tool* t = new Tool;
	if (type == axe) {
		t->name = "Hache";
		t->desc = "Outil. Utilisé pour couper des arbres.";
		t->pos_in_texture_map = {3, 0};
		return t;
	}
	if (type == hoe) {
		t->name = "Faux";
		t->desc = "Outil. Utilisé pour préparer la terre pour l'agriculture.";
		t->pos_in_texture_map = {4, 0};
		return t;
	}

	t->name = "Item was not a Tool";
	return t;
}

Bowl* make_bowl(ItemType type) { 
	Bowl* b = new Bowl;
	if (type == bowl) {
		b->name = "Bol";
		b->desc = "Utilisé pour récolter de l'eau, la boire ou arroser les plants.";
		b->UpdatePosInTextureMap();
		return b;
	}

	b->name = "Item was not a Bowl";
	return b;
}

any* make_any(ItemType type) {
	any* a = new any;
	if (type == wood) {
		a->name = "Bois";
		a->desc = "Utilisé pour construire d'autres objets.";
		a->pos_in_texture_map = {1,0};
		return a;
	}

	a->name = "Probably missing a condition in make_any!";
	return a;
}

ItemType Item::getItemTypeByName(const std::string& name) 
{
	if (name == "Banane")				return banana;
	if (name == "Bois")					return wood;
	if (name == "Pelure de banane")		return banana_peel;
	if (name == "Hache")				return axe;
	if (name == "Faux")					return hoe;
	if (name == "Bol")					return bowl;
	if (name == "Pomme")				return apple;
	if (name == "Coeur de pomme")		return apple_core;
	if (name == "Carotte")				return carrot;
	if (name == "Bout de carotte")		return carrot_top;

	cerr << "item name \"" << name << "\" is not valid." << endl;
	return ItemType(-1);
}

int Manager::CreateItem(ItemType type)
{
	if (IsFood(type)) {
		auto f = make_food(type);
		items[last_id] = f;
		foods[last_id] = f;
	}
	else if (IsBioJunk(type)) {
		auto bj = make_bio_junk(type);
		items[last_id] = bj;
		bio_junks[last_id] = bj;
	}
	else if (IsTool(type)) {
		auto t = make_tool(type);
		items[last_id] = t;
		tools[last_id] = t;
	}
	else if (IsBowl(type)) {
		auto b = make_bowl(type);
		items[last_id] = b;
		bowls[last_id] = b;
	}
	else {
		auto a = make_any(type);
		items[last_id] = a;
	}
	
	++last_id;
	return last_id-1;
}

void Manager::DeleteItem(int id)
{
	if (items.count(id)) {
		auto item = getAny(id);
		ItemType type(getItemTypeByName(item->name));

		if (IsFood(type)) {
			foods[id] = nullptr;
		}
		else if (IsBioJunk(type)) {
			bio_junks[id] = nullptr;
		}
		else if (IsTool(type)) {
			tools[id] = nullptr;
		}
		else if (IsBowl(type)) {
			bowls[id] = nullptr;
		}

		items[id] = nullptr;
		delete item;

	}
	else {
		cerr << "Can not delete item (id: " << id << "), since it does not exist" << endl;
	}
}

