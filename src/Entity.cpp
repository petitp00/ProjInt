#include "Entity.h"
#include "ResourceManager.h"

#include "Globals.h"

#include <iostream>
using namespace std;

int Entity::last_id = 0;

std::vector<std::string> Entity::getSavedData()
{
	return std::vector<std::string>();
}

GameObject::GameObject(int variation, unsigned long flags, std::vector<std::string> const & saved_data):
	Entity({0,0}, {0,0}, flags), variation(variation) {}

GameObject::GameObject(unsigned long flags, std::vector<std::string> const & saved_data):
	Entity({0,0}, {0,0}, flags)
{
	type = GAME_OBJECT;
	variation = stoi(saved_data[0]);
}

void GameObject::Init()
{
	sprite.setTexture(ResourceManager::getTexture(cinfo.texture_name));
	sprite.setTextureRect(cinfo.texture_rect);
	sprite.setScale(scale, scale);
	sprite.setPosition(pos);
	size = vec2(cinfo.texture_rect.width * scale, cinfo.texture_rect.height*scale);
}

void GameObject::setCoordsInfo(CoordsInfo ci)
{
	cinfo = ci;
	cinfo.collision_rect.left -= cinfo.texture_rect.left;
	cinfo.collision_rect.top -= cinfo.texture_rect.top;

	cinfo.collision_rect.left = int(cinfo.collision_rect.left * scale);
	cinfo.collision_rect.top = int(cinfo.collision_rect.top * scale);
	cinfo.collision_rect.width = int(cinfo.collision_rect.width * scale);
	cinfo.collision_rect.height = int(cinfo.collision_rect.height * scale);
}

sf::FloatRect const GameObject::getCollisionBox()
{
	return sf::FloatRect(pos.x + cinfo.collision_rect.left, pos.y + cinfo.collision_rect.top, float(cinfo.collision_rect.width), float(cinfo.collision_rect.height));
}

ItemObject::ItemObject(int item_id, unsigned long flags, std::vector<std::string> const & saved_data) :
	item_id(item_id), GameObject(0, flags, saved_data)
{
	type = ITEM;
	cinfo.texture_name = Item::texture_map_file;
}

TreeObj::TreeObj(Type type, vec2 pos, vec2 size, unsigned long flags, const std::vector<std::string>& saved_data):
	GameObject(0, flags, saved_data)
{
	this->type = type;
	this->pos = pos;
	this->size = size;
	this->flags = flags;

	scale = 1.f;

	if (saved_data.size() != 0) {
		type = getEntityTypeFromString(saved_data[0]);
		growth_level = atoi(saved_data[1].c_str());
		string so = saved_data[2]; // sprite_origin
		cinfo.texture_name = saved_data[3];
		
		Init();
	}
}

void TreeObj::Init()
{
	sprite.setTexture(ResourceManager::getTexture(cinfo.texture_name));
	sprite.setTextureRect(cinfo.texture_rect);
	sprite.setScale(scale, scale);
	sprite.setPosition(pos);
	size = vec2(vec2i(cinfo.texture_rect.left, cinfo.texture_rect.top));
}

void TreeObj::Update(float dt)
{
}

void TreeObj::setGrowthLevel(int level)
{
	growth_level = level;

	if (type == APPLE_TREE) {
		if (level == 0) {
			size = {13, 20};
			flags = NO_FLAG;
		}
		else if (level == 1) {
			//sprite_origin = 
		}
	}

	Init();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		sprite.setTextureRect({vec2i{int(frame*frame_size.x),0}, vec2i(frame_size)});
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
	sprite.setTextureRect({vec2i{0,0}, vec2i(frame_size)});
	sprite.setScale(scale);
}

Player::Player() : Entity()
{
	type = PLAYER;
	Init();
}

Player::Player(vec2 pos, vec2 size, unsigned long flags, std::vector<std::string> const & saved_data) :
	Entity(pos, size, flags, saved_data)
{
	type = PLAYER;
	Init();
}

void Player::Init()
{
	float scale(1.5f);

	size= vec2(36, 36) * scale;

	anim_comp.tileset = &ResourceManager::getTexture("Placeholders/player.png");
	anim_comp.scale = vec2(scale, scale);
	anim_comp.nb_of_frames = 4;
	anim_comp.entity_pos = &pos;
	anim_comp.entity_box_texture_pos ={20,58};
	anim_comp.frame_time = sf::seconds(0.1f);

	anim_comp.Init();
}

void Player::Update(float dt)
{
	if		(sf::Keyboard::isKeyPressed(controls->get("Haut"))) movement.y = -1;
	else if (sf::Keyboard::isKeyPressed(controls->get("Bas"))) movement.y = 1;
	else movement.y = 0;

	if		(sf::Keyboard::isKeyPressed(controls->get("Gauche"))) movement.x = -1;
	else if (sf::Keyboard::isKeyPressed(controls->get("Droite"))) movement.x = 1;
	else movement.x = 0;

	if (movement != vec2(0, 0)) {
		movement = normalize(movement);
		movement.y *= 0.92f;
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

void Player::DoCollisions(std::vector<Entity*>& entities, int entity_move_id)
{
	for (auto e : entities) {
		if (e->HasFlag(SOLID) && e->getId() != entity_move_id) {
			auto ebox = e->getCollisionBox();
			vec2 epos = {ebox.left, ebox.top};
			vec2 esize ={ebox.width, ebox.height};
		

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

void Player::setPos(vec2 pos)
{
	this->pos = pos;
	anim_comp.Update();
}
