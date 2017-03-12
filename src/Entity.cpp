#include "Entity.h"
#include "ResourceManager.h"

#include "Game.h"

#include <iostream>
using namespace std;

int Entity::last_id = 0;

std::vector<std::string> Entity::getSavedData()
{
	return std::vector<std::string>();
}

ComplexGameObject::ComplexGameObject(std::string texture_name, unsigned long flags, std::vector<std::string> const & saved_data):
	Entity({0,0}, {0,0}, flags), texture_name(texture_name)
{
	type = GAME_OBJECT;
}

ComplexGameObject::ComplexGameObject(unsigned long flags, std::vector<std::string> const & saved_data):
	Entity({0,0}, {0,0}, flags)
{
	type = GAME_OBJECT;
}

void ComplexGameObject::Init()
{
	sprite.setTexture(ResourceManager::getTexture(texture_name));
	sprite.setOrigin(sprite_origin);
	sprite.setScale(scale, scale);
	sprite.setPosition(pos);
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
		movement.y *= 0.85f;
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
	target.draw(anim_comp.sprite);
}

void Player::DoCollisions(std::vector<Entity*>& entities)
{
	for (auto e : entities) {
		if (e->HasFlag(SOLID)) {
			auto ebox = e->getCollisionBox();
			sf::Vector2f epos = {ebox.left, ebox.top};
			sf::Vector2f esize ={ebox.width, ebox.height};
		

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
}

void Player::setPos(sf::Vector2f pos)
{
	this->pos = pos;
	anim_comp.Update();
}
