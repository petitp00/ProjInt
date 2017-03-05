#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <string>
#include <vector>

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


class Entity
{
public:
	enum Type {
		ENTITY,
		PLAYER,
		GAME_OBJECT
	};

	Entity()=default;
	Entity(sf::Vector2f pos, sf::Vector2f size, unsigned long flags=NO_FLAG,
		   std::vector<std::string> const& saved_data={}) {
		this->pos = pos;
		this->size = size;
		this->flags = flags;
	}
	virtual ~Entity() = 0 {}

	virtual void Update(float dt) {};
	virtual void Render(sf::RenderTarget& target) = 0;

	void setDead(bool dead) { this->dead = dead; }

	virtual std::vector<std::string> getSavedData();
	sf::Vector2f const& getPos() { return pos; }
	sf::Vector2f const& getSize() { return size; }
	bool getDead() { return dead; }
	Type getType() { return type; }
	unsigned long getFlags() { return flags; }

	void AddFlag(unsigned long flag) { flags |= flag; }
	bool HasFlag(unsigned long flag) { return (flags & flag) == flag; }

protected:
	sf::Vector2f pos;
	sf::Vector2f size;
	bool dead = false;
	Type type = ENTITY;
	unsigned long flags;
};

class GameObject : public Entity
{
public:
	GameObject()=default;
	GameObject(sf::Vector2f pos, std::string texture_name, sf::Vector2f size={ 0,0 },
			   unsigned long flags=NO_FLAG,
			   std::vector<std::string> const& saved_data={});
	GameObject(sf::Vector2f pos, sf::Vector2f size, unsigned long flags=NO_FLAG,
			   std::vector<std::string> const& saved_data={});

	void Render(sf::RenderTarget& target) override { target.draw(sprite); }

	std::vector<std::string> getSavedData() override { return {texture_name}; }

private:
	sf::Sprite sprite;
	std::string texture_name;
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

private:
	Controls* controls = nullptr;

	sf::Sprite sprite; // replace with animation component

	sf::Vector2f movement;
	float walk_speed = 0.3f;
};