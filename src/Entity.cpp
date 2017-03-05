#include "Entity.h"
#include "ResourceManager.h"

#include "Game.h"

#include <iostream>
using namespace std;

GameObject::GameObject(sf::Vector2f pos, std::string texture_name, sf::Vector2f size, unsigned long flags, std::vector<std::string> const& saved_data) :
	Entity(pos, size, flags), texture_name(texture_name)
{
	type = GAME_OBJECT;


	sprite.setTexture(ResourceManager::getTexture(texture_name));
	sprite.setPosition(pos);

	auto b = sprite.getLocalBounds();

	if (size == sf::Vector2f{0, 0}) { this->size ={b.width, b.height}; }
	else { sprite.setScale(size.x / b.width, size.y / b.height); }
}

GameObject::GameObject(sf::Vector2f pos, sf::Vector2f size, unsigned long flags, std::vector<std::string> const & saved_data) :
	Entity(pos, size, flags)
{
	type = GAME_OBJECT;

	if (saved_data.size() == 1) {
		texture_name = saved_data[0];
		sprite.setTexture(ResourceManager::getTexture(saved_data[0]));
		sprite.setPosition(pos);

		auto b = sprite.getLocalBounds();
		sprite.setScale(size.x / b.width, size.y / b.height);
	}
	else {
		cerr << "Wrong number of saved_data. Needs: 1, given: " << saved_data.size() << endl;
		cerr << "ERROR: Trying to create a game object without texture name in saved_data" << endl;
	}
}

Player::Player() : Entity()
{
	type = PLAYER;
	Init();
	auto b = sprite.getLocalBounds();
	size={b.width, b.height};
}

Player::Player(sf::Vector2f pos, sf::Vector2f size, unsigned long flags, std::vector<std::string> const & saved_data) :
	Entity(pos, size, flags, saved_data)
{
	type = PLAYER;
	Init();
}

void Player::Init()
{
	sprite.setTexture(ResourceManager::getTexture("box3.png"));
	sprite.setPosition(pos);
}

void Player::Update(float dt)
{
	if (sf::Keyboard::isKeyPressed(controls->get("Haut"))) movement.y = -1;
	else if (sf::Keyboard::isKeyPressed(controls->get("Bas"))) movement.y = 1;
	else movement.y = 0;

	if (sf::Keyboard::isKeyPressed(controls->get("Gauche"))) movement.x = -1;
	else if (sf::Keyboard::isKeyPressed(controls->get("Droite"))) movement.x = 1;
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

			if (movement.x != 0 && pos.y + size.y > epos.y && pos.y < epos.y + esize.y) {
				if (pos.x + size.x >= epos.x && pos.x + size.x <= epos.x + tolerance
				 && pos.x <= epos.x && movement.x > 0) {
					pos.x = epos.x - size.x;
					movement.x = 0;
				}
				else if (pos.x <= epos.x + esize.x && pos.x >= epos.x + esize.x - tolerance
					  && pos.x + size.x >= epos.x + esize.x && movement.x < 0) {
					pos.x = epos.x + esize.x;
					movement.x = 0;
				}
			}

			if (movement.y != 0 && pos.x + size.x > epos.x && pos.x < epos.x + esize.x) {
				if (pos.y + size.y >= epos.y && pos.y + size.y <= epos.y + tolerance
				 && pos.y <= epos.y && movement.y > 0) {
					pos.y = epos.y - size.y;
					movement.y = 0;
				}
				else if (pos.y <= epos.y + esize.y && pos.y >= epos.y + esize.y - tolerance
					  && pos.y + size.y >= epos.y + esize.y && movement.y < 0) {
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

std::vector<std::string> Entity::getSavedData()
{
	return std::vector<std::string>();
}
