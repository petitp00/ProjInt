#include "Entity.h"
#include "ResourceManager.h"

#include "Game.h"

#include <iostream>
using namespace std;

GameObject::GameObject(sf::Vector2f pos, std::string texture_name, sf::Vector2f size, unsigned long flags) :
	Entity(pos, size, flags)
{
	sprite.setTexture(ResourceManager::getTexture(texture_name));
	sprite.setPosition(pos);

	auto b = sprite.getLocalBounds();

	if (size == sf::Vector2f{0, 0}) { this->size ={b.width, b.height}; }
	else { sprite.setScale(size.x / b.width, size.y / b.height); }
}

Player::Player() : Entity()
{
	Init();
}

void Player::Init()
{
	sprite.setTexture(ResourceManager::getTexture("box3.png"));
	auto b = sprite.getLocalBounds();
	size ={b.width, b.height};
}

void Player::Update(float dt)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) movement.y = -1;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) movement.y = 1;
	else movement.y = 0;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) movement.x = -1;
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) movement.x = 1;
	else movement.x = 0;

	if (movement != sf::Vector2f(0, 0)) {
		movement = normalize(movement);
	}
}

void Player::Render(sf::RenderTarget & target)
{
	target.draw(sprite);
}

void Player::DoCollisions(std::vector<Entity*>& entities)
{
	for (auto e : entities) {
		if (e->HasFlag(SOLID)) {
			auto epos = e->getPos();
			auto esize = e->getSize();

			float tolerance = 10;
			
			if (movement.x != 0
			 && pos.y + size.y > epos.y
			 && pos.y < epos.y + esize.y) {

				if (pos.x + size.x >= epos.x
				 && pos.x + size.x <= epos.x + tolerance
				 && pos.x <= epos.x
				 && movement.x > 0) {

					pos.x = epos.x - size.x;
					movement.x = 0;
				}
				else if (pos.x <= epos.x + esize.x
					  && pos.x >= epos.x + esize.x - tolerance
					  && pos.x + size.x >= epos.x + esize.x
					  && movement.x < 0) {

					pos.x = epos.x + esize.x;
					movement.x = 0;
				}
			}

			if (movement.y != 0
			 && pos.x + size.x > epos.x
			 && pos.x < epos.x + esize.x) {

				if (pos.y + size.y >= epos.y
				 && pos.y + size.y <= epos.y + tolerance
				 && pos.y <= epos.y
				 && movement.y > 0) {

					pos.y = epos.y - size.y;
					movement.y = 0;
				}
				else if (pos.y <= epos.y + esize.y
					  && pos.y >= epos.y + esize.y - tolerance
					  && pos.y + size.y >= epos.y + esize.y
					  && movement.y < 0) {

					pos.y = epos.y + esize.y;
					movement.y = 0;
				}
			}

		}
	}
}

void Player::DoMovement(float dt)
{
	pos += movement * walk_speed * dt;
	sprite.setPosition(pos);
}
