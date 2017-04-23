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

// FLAGS
#define NO_FLAG 0
#define IMMORTAL 1 // never dies (should be sorted at head of vector)
#define	SOLID 2 // can't walk through (has collisions)
#define PICKUP 4 // can be picked up

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
	TREE = 6,
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
	case TREE:			return "TREE";
	case ITEM:			return "ITEM";
	case APPLE_TREE:	return "APPLE_TREE";
	case BANANA_TREE:	return "BANANA_TREE";
	case HUT:			return "HUT";
	case Type::ERROR:	return "ERROR";
	default:			return "UNKNOWN. (Maybe you forgot to add it to getEntityTypeString() ?";
	}
}

static Type getEntityTypeFromString(const std::string& str) {
	if (str == "ENTITY")		return ENTITY;
	if (str == "PLAYER")		return PLAYER;
	if (str == "GAME_OBJECT")	return GAME_OBJECT;
	if (str == "ROCK")			return ROCK;
	if (str == "BUSH")			return BUSH;
	if (str == "TREE")			return TREE;
	if (str == "ITEM")			return ITEM;
	if (str == "APPLE_TREE")	return APPLE_TREE;
	if (str == "BANANA_TREE")	return BANANA_TREE;
	if (str == "HUT")			return HUT;
	return Type::ERROR;
}

class Entity
{
public:

	Entity() {
		++last_id;
		id = last_id;
	}
	Entity(vec2 pos, vec2 size, unsigned long flags=NO_FLAG,
		   std::vector<std::string> const& saved_data={}) {
		this->pos = pos;
		this->size = size;
		this->flags = flags;

		++last_id;
		id = last_id;
	}
	virtual ~Entity() = 0 {}

	virtual void Update(float dt) {};
	virtual void Render(sf::RenderTarget& target) = 0;

	virtual void setPos(vec2 pos) { this->pos = pos; }
	void setDead(bool dead) { this->dead = dead; }

	virtual std::vector<std::string> getSavedData();
	vec2 const& getPos() { return pos; }
	virtual vec2 getOrigin() { return {0,0}; }
	vec2 const& getSize() { return size; }
	virtual sf::FloatRect const getCollisionBox() { return {pos,size}; }
	bool getDead() { return dead; }
	Type getType() { return type; }
	unsigned long getFlags() { return flags; }
	int getId() { return id; }

	void AddFlag(unsigned long flag) { flags |= flag; }
	bool HasFlag(unsigned long flag) { return (flags & flag) == flag; }

protected:
	vec2 pos;
	vec2 size;
	bool dead = false;
	Type type = ENTITY;
	unsigned long flags;

	int id;
	static int last_id;
};

class GameObject : public Entity
{
	friend GameObject* make_rock(vec2 pos, int variation);
	friend GameObject* make_hut(vec2 pos);
public:
	GameObject()=default;
	GameObject(int variation, unsigned long flags=NO_FLAG,
					  std::vector<std::string> const& saved_data={});
	GameObject(unsigned long flags,
					  std::vector<std::string> const& saved_data={}); // for loading

	void Init(); // call this after members are set
	void Render(sf::RenderTarget& target) override { 
		target.draw(sprite);

		sf::RectangleShape shape(vec2(getCollisionBox().width, getCollisionBox().height));
		shape.setPosition(vec2(getCollisionBox().left, getCollisionBox().top));
		shape.setFillColor(sf::Color(255, 0, 0, 60));
		//target.draw(shape);

		shape.setSize(size);
		shape.setPosition(pos);
		shape.setFillColor(sf::Color(0, 255, 0, 60));
		//target.draw(shape);
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
	ItemObject()=default;
	ItemObject(int item_id, unsigned long flags=NO_FLAG,
					  std::vector<std::string> const& saved_data={});
	
	ItemObject(unsigned long flags,
					  std::vector<std::string> const& saved_data={}) = delete; // for loading

	std::vector<std::string> getSavedData() override {
		auto item = Item::Manager::getAny(item_id);
		return { // save with item id (referring to another part of the save file) ???????
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
	friend TreeObj* make_tree_obj(
		Type type, int variation, vec2 pos, 
		std::vector<std::string> saved_data);// = std::vector<std::string>());
public:
	TreeObj()=default;
	TreeObj(Type type, vec2 pos, vec2 size, unsigned long flags = SOLID,
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
	Player(vec2 pos, vec2 size, unsigned long flags=NO_FLAG,
		   std::vector<std::string> const& saved_data={});

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

	auto rock = new GameObject(variation, SOLID|IMMORTAL);

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

	rock->type = ROCK;
	rock->pos = pos;
	rock->scale = 2.f;
	rock->setCoordsInfo(info);
	rock->Init();
	return rock;
}

static GameObject* make_hut(vec2 pos = {0,0}) {
	auto hut = new GameObject(0, SOLID);
	CoordsInfo i = getCoordsInfo("hut");
	hut->type = HUT;
	hut->pos = pos;
	hut->scale = 2.f;
	hut->setCoordsInfo(i);
	hut->Init();
	return hut;
}

static TreeObj* make_tree_obj(
	Type type, int variation = 5, vec2 pos= {0,0},
	std::vector<std::string> saved_data = std::vector<std::string>()) {

	auto tree = new TreeObj(type, pos, {0,0}, SOLID, saved_data);
	tree->setGrowthLevel(variation);
	tree->Init();
	return tree;
}

static ItemObject* make_item(int id, vec2 pos = {0,0}) {
	float ts = Item::items_texture_size;
	auto i = new ItemObject(id, SOLID);
	i->type = ITEM;
	i->pos = pos;
	auto scale = 1.f;
	i->size = {ts*scale, ts*scale};
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
	
