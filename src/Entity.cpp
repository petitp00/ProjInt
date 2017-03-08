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

std::vector<std::string> Entity::getSavedData()
{
	return std::vector<std::string>();
}

void AnimationComponent::Update()
{
	sprite.setPosition(*entity_pos);
	if (clock.getElapsedTime() > frame_time) {
		clock.restart();
		if (frame < nb_of_frames-1) {
			++frame;
		}
		else {
			frame = 0;
		}
		sprite.setTextureRect({sf::Vector2i{int(frame*frame_size.x),0}, sf::Vector2i(frame_size)});
		if (flip) {
			sprite.setOrigin(frame_size.x - entity_box_texture_pos.x, entity_box_texture_pos.y);
			sprite.setScale(-scale.x, scale.y);
		}
		else {
	sprite.setOrigin(entity_box_texture_pos);
			sprite.setScale(scale.x, scale.y);
		}
	}
}

void AnimationComponent::Init()
{
	sprite.setTexture(*tileset);
	sprite.setOrigin(entity_box_texture_pos);
	sprite.setPosition(*entity_pos);

	frame_size.x = sprite.getLocalBounds().width / float(nb_of_frames);
	frame_size.y = sprite.getLocalBounds().height;
	sprite.setTextureRect({sf::Vector2i{0,0}, sf::Vector2i(frame_size)});
	sprite.setScale(scale);
}

Player::Player() : Entity()
{
	type = PLAYER;
	Init();
	auto b = sprite.getLocalBounds();
	//size={b.width, b.height};
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

	float scale(1.5f);

	size= sf::Vector2f(36, 36) * scale;

	anim_comp.tileset = &ResourceManager::getTexture("Placeholders/player.png");
	anim_comp.scale = sf::Vector2f(scale, scale);
	anim_comp.nb_of_frames = 4;
	anim_comp.entity_pos = &pos;
	anim_comp.entity_box_texture_pos ={20,58};
	anim_comp.frame_time = sf::seconds(0.1f);

	anim_comp.Init();
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
		anim_comp.Update();
	}

	if (movement.x > 0) {
		anim_comp.flip = false;
	}
	else if (movement.x < 0) {
		anim_comp.flip = true;
	}

}

void Player::Render(sf::RenderTarget & target)
{
	//target.draw(sprite);
	target.draw(anim_comp.sprite);
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
