#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <string>
#include <vector>

// FLAGS
#define NO_FLAG 0
#define IMMORTAL 1 // never dies (should be sorted at head of vector)
#define	SOLID 2 // can't walk through (has collisions)
#define PICKUP 4 // can be picked up

class Entity
{
public:
	Entity()=default;
	Entity(sf::Vector2f pos, sf::Vector2f size, unsigned long flags=NO_FLAG) {
		this->pos = pos;
		this->size = size;
		this->flags = flags;
	}
	virtual ~Entity() = 0 {}

	virtual void Update(float dt) {};
	virtual void Render(sf::RenderTarget& target) = 0;

	void setDead(bool dead) { this->dead = dead; }

	sf::Vector2f const& getPos() { return pos; }
	sf::Vector2f const& getSize() { return size; }
	bool getDead() { return dead; }

	void AddFlag(unsigned long flag) { flags |= flag; }
	bool HasFlag(unsigned long flag) { return (flags & flag) == flag; }

protected:
	sf::Vector2f pos;
	sf::Vector2f size;
	bool dead = false;
	unsigned long flags;
};

class GameObject : public Entity
{
public:
	GameObject()=default;
	GameObject(sf::Vector2f pos,
			   std::string texture_name,
			   sf::Vector2f size={ 0,0 },
			   unsigned long flags=NO_FLAG);

	void Render(sf::RenderTarget& target) override { target.draw(sprite); }

private:
	sf::Sprite sprite;
};

class Player : public Entity
{
public:
	Player();

	void Init();
	
	void Update(float dt) override;
	void Render(sf::RenderTarget& target) override;

	void DoCollisions(std::vector<Entity*>& entities);
	void DoMovement(float dt);

private:
	sf::Sprite sprite; // replace with animation component

	sf::Vector2f movement;
	float walk_speed = 0.3f;
};