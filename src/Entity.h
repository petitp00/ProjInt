#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <string>

// FLAGS
#define NO_FLAG 0
#define SOLID 1 // can't walk through (has collisions)
#define	STATIC 2 // does not move
#define IMMORTAL 4 // never dies (should be sorted at head of vector)

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
	GameObject(sf::Vector2f pos, std::string texture_name, sf::Vector2f size={ 0,0 }, unsigned long flags=NO_FLAG);

	void Render(sf::RenderTarget& target) override { target.draw(sprite); }

private:
	sf::Sprite sprite;
};