#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>

#include <string>
#include <vector>
#include <iostream>

#include "Items.h"

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
	return Type::ERROR;
}

class Entity
{
public:

	Entity() {
		++last_id;
		id = last_id;
	}
	Entity(sf::Vector2f pos, sf::Vector2f size, unsigned long flags=NO_FLAG,
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

	virtual void setPos(sf::Vector2f pos) { this->pos = pos; }
	void setDead(bool dead) { this->dead = dead; }

	virtual std::vector<std::string> getSavedData();
	sf::Vector2f const& getPos() { return pos; }
	virtual sf::Vector2f getOrigin() { return {0,0}; }
	sf::Vector2f const& getSize() { return size; }
	virtual sf::FloatRect const getCollisionBox() { return {pos,size}; }
	bool getDead() { return dead; }
	Type getType() { return type; }
	unsigned long getFlags() { return flags; }
	int getId() { return id; }

	void AddFlag(unsigned long flag) { flags |= flag; }
	bool HasFlag(unsigned long flag) { return (flags & flag) == flag; }

protected:
	sf::Vector2f pos;
	sf::Vector2f size;
	bool dead = false;
	Type type = ENTITY;
	unsigned long flags;

	int id;
	static int last_id;
};

class GameObject : public Entity
{
	friend GameObject* make_rock(sf::Vector2f pos);
	friend GameObject* make_bush(sf::Vector2f pos);
	friend GameObject* make_tree(sf::Vector2f pos);
public:
	GameObject()=default;
	GameObject(std::string texture_name, unsigned long flags=NO_FLAG,
					  std::vector<std::string> const& saved_data={});
	GameObject(unsigned long flags,
					  std::vector<std::string> const& saved_data={}); // for loading

	void Init(); // call this after members are set
	void Render(sf::RenderTarget& target) override { target.draw(sprite); }

	void setPos(sf::Vector2f pos) override { Entity::setPos(pos); sprite.setPosition(pos); }
	sf::Vector2f getOrigin() override { return sprite_origin*scale; }
	std::vector<std::string> getSavedData() override { return {
			std::to_string(pos.x) +" "+ std::to_string(pos.y),
			std::to_string(size.x) +" "+ std::to_string(size.y),
			std::to_string(sprite_origin.x) +" "+ std::to_string(sprite_origin.y),
			texture_name,
		}; }
protected:
	sf::Sprite sprite;
	sf::Vector2f sprite_origin ={0,0};
	std::string texture_name = "";
	float scale = 1.f;
};

class ItemObject : public GameObject
{
	friend ItemObject* make_item(int id, sf::Vector2f pos);
public:
	ItemObject()=default;
	ItemObject(int item_id, std::string texture_name, unsigned long flags=NO_FLAG,
					  std::vector<std::string> const& saved_data={});
	
	ItemObject(unsigned long flags,
					  std::vector<std::string> const& saved_data={}) = delete; // for loading

	std::vector<std::string> getSavedData() override {
		auto item = Item::Manager::getAny(item_id);
		return { // save with item id (referring to another part of the save file) ???????
			item->name,
			std::to_string(pos.x) +" "+ std::to_string(pos.y),
			std::to_string(size.x) +" "+ std::to_string(size.y),
			std::to_string(sprite_origin.x) +" "+ std::to_string(sprite_origin.y),
			texture_name,


		};
	}
	sf::FloatRect const getCollisionBox() override { return {pos+size/4.f,size/2.f}; }
	int getItemId() { return item_id; }

private:
	int item_id;

};

class AnimationComponent
{
public:
	void Update();
	void Init();

	sf::Vector2f* entity_pos; // not the same as the sprite's pos
	sf::Vector2f entity_box_texture_pos; // where the collision box of the entity starts in the texture
	sf::Vector2f frame_size;
	sf::Vector2f scale;
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
	Player(sf::Vector2f pos, sf::Vector2f size, unsigned long flags=NO_FLAG,
		   std::vector<std::string> const& saved_data={});

	void Init();
	void Update(float dt) override;
	void Render(sf::RenderTarget& target) override;
	void DoCollisions(std::vector<Entity*>& entities);
	void DoMovement(float dt);
	void setControls(Controls* controls) { this->controls = controls; }
	void setPos(sf::Vector2f pos) override;

private:
	Controls* controls = nullptr;

	AnimationComponent anim_comp;

	sf::Vector2f movement;
	float walk_speed = 0.3f;
};

/*
	--- ### --- ### --- MAKE ENTITY FUNCTIONS --- ### --- ### --- ### ---
*/

static Entity* make_entity(Type type, sf::Vector2f pos={0,0}) {
	Entity* e = nullptr;

	if		(type == ROCK)	e = make_rock(pos);
	else if (type == BUSH)	e = make_bush(pos);
	else if (type == TREE)	e = make_tree(pos);

	return e;
}

static GameObject* make_rock(sf::Vector2f pos ={0,0}) {
	auto rock = new GameObject("Placeholders/rock.png", SOLID|IMMORTAL);
	rock->type = ROCK;
	rock->pos = pos;
	rock->sprite_origin = {14, 34};
	auto scale = 2.f;
	rock->size ={40.f*scale, 20.f*scale};
	rock->scale = scale;
	rock->Init();
	return rock;
}

static GameObject* make_bush(sf::Vector2f pos ={0,0}) {
	auto bush = new GameObject("Placeholders/bush.png", NO_FLAG);
	bush->type = BUSH;
	bush->pos = pos;
	bush->sprite_origin ={0,0};
	auto scale = 2.f;
	bush->size ={46*scale, 36*scale};
	bush->scale = scale;
	bush->Init();
	return bush;
}

static GameObject* make_tree(sf::Vector2f pos= {0,0}) {
	auto tree = new GameObject("Placeholders/tree.png", SOLID);
	tree->type = TREE;
	tree->pos = pos;
	tree->sprite_origin = {53,100};
	auto scale = 4.f;
	tree->size = {13*scale,9*scale};
	tree->scale = scale;
	tree->Init();
	return tree;
}

static ItemObject* make_item(int id, sf::Vector2f pos = {0,0}) {
	float ts = Item::items_texture_size;
	auto i = new ItemObject(id, Item::texture_map_file, SOLID);
	i->type = ITEM;
	i->pos = pos;
	auto scale = 3.f;
	i->size = {ts*scale, ts*scale};
	i->scale = scale;
	i->Init();

	auto item = Item::Manager::getAny(id);

	i->sprite.setTextureRect(
		{int(item->pos_in_texture_map.x*ts), int(item->pos_in_texture_map.y*ts), int(ts), int(ts)});
	return i;
}
	
