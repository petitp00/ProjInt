#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <string>
#include <vector>
#include <iostream>

#include "Items.h"
#include "Game.h"

struct Controls;

/*
Entity Saving:
	- pos
	- size
	- typeid
	- flags
	- vector of strings of special things to save (got from an overloaded function)???
		+ eg: health of objects
		+ the vector would be passed as an argument in the entity's ctor
*/

/*
NEW GAME OBJECT CHECKLIST

- add to Type enum
- add to getEntityTypeString
- add to getEntityTypeFromString
- make friend function in GameObject
- create make_<type>()
- add to make_entity
- add to World::LoadWorld

*/

enum Type {
	ERROR = 0,
	ENTITY = 1,
	PLAYER = 2,
	GAME_OBJECT = 3,
	ROCK = 4,
	BUSH = 5,
	ITEM = 7,
	APPLE_TREE = 8,
	BANANA_TREE = 9,
	HUT = 10,
};

static std::string getEntityTypeString(Type t) {
	switch (t)
	{
	case ENTITY:		return "ENTITY";
	case PLAYER:		return "PLAYER";
	case GAME_OBJECT:	return "GAME_OBJECT";
	case ROCK:			return "ROCK";
	case BUSH:			return "BUSH";
	case ITEM:			return "ITEM";
	case APPLE_TREE:	return "APPLE_TREE";
	case BANANA_TREE:	return "BANANA_TREE";
	case HUT:			return "HUT";
	case Type::ERROR:	return "ERROR";
	default:			return "UNKNOWN. (Maybe you forgot to add it to getEntityTypeString() ?";
	}
}

static std::string getEntityName(Type t) {
	switch (t)
	{
	case ERROR:			return "Erreur";
	case ENTITY:		return "Entit�";
	case PLAYER:		return "Joueur";
	case GAME_OBJECT:	return "GameObject";
	case ROCK:			return "Roche";
	case BUSH:			return "Buisson";
	case ITEM:			return "Item";
	case APPLE_TREE:	return "Pommier";
	case BANANA_TREE:	return "Bananier";
	case HUT:			return "Cabane";
	default:			return "case missing in getEntityName()";
	}
}

static Type getEntityTypeFromString(const std::string& str) {
	if (str == "ENTITY")		return ENTITY;
	if (str == "PLAYER")		return PLAYER;
	if (str == "GAME_OBJECT")	return GAME_OBJECT;
	if (str == "ROCK")			return ROCK;
	if (str == "BUSH")			return BUSH;
	if (str == "ITEM")			return ITEM;
	if (str == "APPLE_TREE")	return APPLE_TREE;
	if (str == "BANANA_TREE")	return BANANA_TREE;
	if (str == "HUT")			return HUT;
	return Type::ERROR;
}

/*
	Base class, is pure virtual
*/
class Entity
{
public:
	Entity() {
		++last_id;
		id = last_id;
	}
	Entity(Type type, vec2 pos, vec2 size, std::vector<std::string> const& saved_data={}) {
		this->type = type;
		this->pos = pos;
		this->size = size;
		++last_id;
		id = last_id;
	}
	virtual ~Entity() = 0 {}
	virtual void Update(float dt) {};
	virtual void Render(sf::RenderTarget& target) = 0;

	// Setters
	virtual void setPos(vec2 pos) { this->pos = pos; }
	void setDead(bool dead) { this->dead = dead; }

	// Getters
	Type		getType()	{ return type; }
	const vec2& getPos()	{ return pos; }
	const vec2& getSize()	{ return size; }
	bool		getDead()	{ return dead; }
	bool		getSolid()	{ return solid; }
	int			getId()		{ return id; }
	virtual sf::FloatRect const getCollisionBox() { return {pos,size}; }

	virtual std::vector<std::string> getSavedData();
	virtual std::string getHoverInfo();

protected:
	Type type = ENTITY;
	vec2 pos;
	vec2 size;

	bool dead = false;
	bool solid = true;

	int id;
	static int last_id;
};

class GameObject : public Entity
{
	friend GameObject* make_rock(vec2 pos, int variation);
	friend GameObject* make_hut(vec2 pos);
public:
	GameObject(Type type, int variation, vec2 pos = vec2(0, 0), vec2 size = vec2(0, 0), const std::vector<std::string>& saved_data={});

	void Init(); // call this after members are set
	void Render(sf::RenderTarget& target) override { 
		target.draw(sprite);
	}

	void setPos(vec2 pos) override { Entity::setPos(pos); sprite.setPosition(pos); }
	void setCoordsInfo(CoordsInfo ci);

	sf::FloatRect const getCollisionBox() override;
	std::vector<std::string> getSavedData() override { return {
		std::to_string(variation)
	}; }
protected:
	sf::Sprite sprite;
	CoordsInfo cinfo;
	float scale;
	int variation = 0;
};

class ItemObject : public GameObject
{
	friend ItemObject* make_item(int id, vec2 pos);
public:
	ItemObject(int item_id, vec2 pos = vec2(0,0), vec2 size = vec2(0,0), std::vector<std::string> const& saved_data={});

	std::vector<std::string> getSavedData() override {
		auto item = Item::Manager::getAny(item_id);
		return {
			item->name, std::to_string(variation)
		};
	}
	sf::FloatRect const getCollisionBox() override { return {pos+size/4.f,size/2.f}; }
	int getItemId() { return item_id; }

private:
	int item_id;
};

class TreeObj : public GameObject
{
	friend TreeObj* make_tree_obj( Type type, int variation, vec2 pos, 
		std::vector<std::string> saved_data);
public:
	TreeObj()=default;
	TreeObj(Type type, vec2 pos,
			const std::vector<std::string>& saved_data = {});

	void Init();
	void Update(float dt) override;

	void Hit() { --hp; }
	bool getChopped() { return hp <= 0; }
	std::vector<std::pair<Item::ItemType,int>> getDroppedItems();
	int getGrowthLevel() { return growth_level; }
	int getFruitsAmount() { return fruits; }
	void TakeOneFruit();// { --fruits; }
	void setFruitAmount(int amount) {fruits = amount;}
	void setGrowthLevel(int level);

	std::vector<std::string> getSavedData() override { return {
		std::to_string(int(type)),
		std::to_string(growth_level),
		std::to_string(hp),
		std::to_string(fruits)
	}; }


private:
	int growth_level = 0; // 1 to 6
	int fruits = 0;
	int hp;
};

class AnimationComponent
{
public:
	void Update();
	void Init();

	vec2* entity_pos; // not the same as the sprite's pos
	vec2 entity_box_texture_pos; // where the collision box of the entity starts in the texture
	vec2 frame_size;
	vec2 scale;
	sf::Sprite sprite;
	sf::Texture* tileset;
	sf::Time frame_time;

	int nb_of_frames;
	int frame = 0;

	bool flip = false;

private:
	sf::Clock clock;
};

class Player : public Entity
{
public:
	Player();
	Player(vec2 pos, vec2 size, std::vector<std::string> const& saved_data={});

	void Init();
	void Update(float dt) override;
	void Render(sf::RenderTarget& target) override;
	void DoCollisions(std::vector<Entity*>& entities, int entity_move_id = -1);
	void DoMovement(float dt);
	void setControls(Controls* controls) { this->controls = controls; }
	void setPos(vec2 pos) override;

private:
	Controls* controls = nullptr;

	AnimationComponent anim_comp;

	vec2 movement;
	float walk_speed = 0.3f;
};

/*
	--- ### --- ### --- MAKE ENTITY FUNCTIONS --- ### --- ### --- ### ---
*/

static Entity* make_entity(
	Type type, vec2 pos= {0,0}, int variation=0,
	std::vector<std::string> saved_data = std::vector<std::string>()) {

	Entity* e = nullptr;

	if (type == ROCK)				e = make_rock(pos, variation);
	else if (type == APPLE_TREE)	e = make_tree_obj(type, variation, pos, saved_data);
	else if (type == BANANA_TREE)	e = make_tree_obj(type, variation, pos, saved_data);
	else if (type == HUT)			e = make_hut(pos);
	return e;
}

static GameObject* make_rock( vec2 pos = {0,0}, int variation = 1) {

	auto rock = new GameObject(ROCK, variation);

	CoordsInfo info;
	if (variation == 1) {
		info = getCoordsInfo("rock1");
	}
	else if (variation == 2) {
		info = getCoordsInfo("rock2");
	}
	else if (true || variation == 3) {
		info = getCoordsInfo("rock3");
	}

	rock->pos = pos;
	rock->scale = 2.f;
	rock->setCoordsInfo(info);
	rock->Init();
	return rock;
}

static GameObject* make_hut(vec2 pos = {0,0}) {
	auto hut = new GameObject(HUT, 0);
	CoordsInfo i = getCoordsInfo("hut");
	hut->pos = pos;
	hut->scale = 2.f;
	hut->setCoordsInfo(i);
	hut->Init();
	return hut;
}

static TreeObj* make_tree_obj(
	Type type, int variation = 5, vec2 pos= {0,0},
	std::vector<std::string> saved_data = std::vector<std::string>()) {

	auto tree = new TreeObj(type, pos, saved_data);
	tree->setGrowthLevel(variation);
	tree->Init();
	return tree;
}

static ItemObject* make_item(int id, vec2 pos = {0,0}) {
	float ts = Item::items_texture_size;

	auto scale = 1.f;
	vec2 size = {ts*scale, ts*scale};

	auto i = new ItemObject(id, pos, size);
	i->scale = scale;

	auto item = Item::Manager::getAny(id);

	i->cinfo.texture_name = Item::texture_map_file;
	i->cinfo.texture_rect = {
		int(item->pos_in_texture_map.x*ts),
		int(item->pos_in_texture_map.y*ts), int(ts), int(ts)};
	i->cinfo.collision_rect = {
		int(item->pos_in_texture_map.x*ts),
		int(item->pos_in_texture_map.y*ts), int(ts), int(ts)};
	i->Init();

	return i;
}
	
