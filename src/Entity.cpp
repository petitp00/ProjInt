#include "Entity.h"
#include "ResourceManager.h"

#include "Globals.h"

#include <sstream>
#include <iostream>
using namespace std;

int Entity::last_id = 0;

std::vector<std::string> Entity::getSavedData()
{
	cout << "Warning: Entity::getSavedData() called." << endl;
	return std::vector<std::string>();
}

std::string Entity::getHoverInfo()
{
	cout << "Warning: Entity::getHoverInfo() called." << endl;
	return std::string();
}

GameObject::GameObject(Type type, int variation, vec2 pos, vec2 size,
	std::vector<std::string> const & saved_data):
	Entity(type, pos, size), variation(variation) {}

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
	cinfo.collision_rect.top  -= cinfo.texture_rect.top;

	cinfo.collision_rect.left	=	int(cinfo.collision_rect.left * scale);
	cinfo.collision_rect.top	=	int(cinfo.collision_rect.top * scale);
	cinfo.collision_rect.width	=	int(cinfo.collision_rect.width * scale);
	cinfo.collision_rect.height	=	int(cinfo.collision_rect.height * scale);
}

sf::FloatRect const GameObject::getCollisionBox()
{
	return sf::FloatRect(
		pos.x + cinfo.collision_rect.left,
		pos.y + cinfo.collision_rect.top,
		float(cinfo.collision_rect.width),
		float(cinfo.collision_rect.height)
	);
}

std::string GameObject::getHoverInfo()
{
	stringstream str;

	if (type == ROCK) {
		str << "Roche\n";
	}
	else if (type == HUT) {
		str << "Cabane" << '\n';
		str << "Clic gauche pour dormir." << '\n'
			<< "Recharge la barre d'énergie et passe la nuit.\n";
	}

	return str.str();
}

ItemObject::ItemObject(int item_id, vec2 pos, vec2 size,
	std::vector<std::string> const & saved_data) :
	item_id(item_id), GameObject(Type::ITEM, 0, pos, size, saved_data)
{
	cinfo.texture_name = Item::texture_map_file;
}

std::string ItemObject::getHoverInfo()
{
	stringstream str;

	auto itype = Item::Manager::getItemType(item_id);
	auto iany = Item::Manager::getAny(item_id);

	str << iany->name << '\n';
	str << iany->desc << '\n';

	if (Item::IsFood(itype)) {
		auto food = Item::Manager::getFood(item_id);
		str << "Pourriture: " << to_string(food->spoil_level) << '%' << '\n';
	}

	return str.str();
}

TreeObj::TreeObj(Type type, vec2 pos, const std::vector<std::string>& saved_data):
	GameObject(type, 0, pos, vec2(0,0), saved_data)
{
	scale = 2.f;

	if (saved_data.size() != 0) {
		pos_adjusted = true;
		type = getEntityTypeFromString(saved_data[0]);
		growth_level = atoi(saved_data[1].c_str());
		variation = growth_level;
		if (saved_data.size() >= 3) {
			hp = atoi(saved_data[2].c_str());
			fruits = min(4, atoi(saved_data[3].c_str()));
		}
	}
	else {
		fruits = -1;
		pos_adjusted = false;
		if (growth_level != 0) {
			setGrowthLevel(growth_level);
			cout << "GROWTH LEVEL IS NOT 0. keep (line: " << __LINE__ << ")" << endl;
		}
	}
}

void TreeObj::Init()
{
	sprite.setTexture(ResourceManager::getTexture(cinfo.texture_name));
	sprite.setTextureRect(cinfo.texture_rect);
	sprite.setScale(scale, scale);
	size = vec2(cinfo.texture_rect.width * scale, cinfo.texture_rect.height*scale);

	if (!pos_adjusted) {
		pos -= vec2(size.x / 2.f, size.y);
		pos_adjusted = true;
	}

	sprite.setPosition(pos);
}

void TreeObj::Update(float dt)
{
}

std::vector<std::pair<Item::ItemType, int>> TreeObj::getDroppedItems()
{
	vector<pair<Item::ItemType, int>> vec;

	if (type == APPLE_TREE) {

		if (growth_level == 3)
			vec.push_back({Item::ItemType::wood, 2});
		else if (growth_level == 4 || growth_level == 5 || growth_level == 6)
			vec.push_back({Item::ItemType::wood, 3});
		if (growth_level == 6)
			vec.push_back({Item::ItemType::apple, fruits});
	}
	else if (type == BANANA_TREE) {
		if (growth_level == 4 || growth_level == 5 || growth_level == 6)
			vec.push_back({Item::ItemType::banana_leaf, 3});
		if (growth_level == 6)
			vec.push_back({Item::ItemType::banana, fruits});
	}

	return vec;
}

void TreeObj::TakeOneFruit()
{
	--fruits;

	if (fruits <= 0) {
		fruits = 0;
		setGrowthLevel(4);
	}
}

void TreeObj::setGrowthLevel(int level)
{
	growth_level = level;


	if (growth_level == 1 || growth_level == 2) {
		hp = 1;
	}
	else if (growth_level == 3) {
		hp = 2;
	}
	else if (growth_level == 4 || growth_level == 5 || growth_level == 6) {
		hp = 3;
	}

	if (growth_level == 6 && fruits == -1) {
		fruits = 4;
	}
	else if (fruits == -1) {
		fruits = 0;
	}

	if (type == APPLE_TREE) {
		cinfo = getCoordsInfo("appletree" + to_string(growth_level));
	}
	else if (type == BANANA_TREE) {
		cinfo = getCoordsInfo("bananatree" + to_string(growth_level));
	}

	setCoordsInfo(cinfo);

	Init();
}

std::string TreeObj::getHoverInfo()
{
	stringstream str;

	auto drop = getDroppedItems();

	if (type == APPLE_TREE) {
		str << "Pommier" << '\n';
		if (drop.size() > 0) {
			str << "Donne " << to_string(drop[0].second) << " bois." << '\n';
		}
	}
	else if (type == BANANA_TREE) {
		str << "Bananier" << '\n';
		if (drop.size() > 0) {
			str << "Donne " << (drop[0].second) << " feuilles de bananier." << '\n';
		}
	}

	str << "Fruits à cueillir: " << to_string(fruits) << '\n';

	return str.str();
}

CarrotPlant::CarrotPlant(vec2 pos, const std::vector<std::string>& saved_data) : 
	GameObject(CARROT_PLANT, 0, pos, vec2(0,0), saved_data)
{
	scale = 2.f;

	if (saved_data.size() != 0) {
		growth_level = float(atoi(saved_data[0].c_str()));
	}
}

void CarrotPlant::Init()
{
	UpdateCinfo();

	sprite.setTexture(ResourceManager::getTexture(cinfo.texture_name));
	sprite.setTextureRect(cinfo.texture_rect);
	sprite.setScale(scale, scale);
	size = vec2(cinfo.texture_rect.width * scale, cinfo.texture_rect.height * scale);
	sprite.setPosition(pos);
}

void CarrotPlant::Update(float dt)
{
}

void CarrotPlant::UpdateCinfo()
{
	int var = 1;
	if (growth_level >= 100) { var = 3; }
	else if (growth_level >= 50) { var = 2; }
	cinfo = getCoordsInfo("carrotplant" + to_string(var));
}

void CarrotPlant::setGrowthLevel(float growth_level)
{
	this->growth_level = growth_level;
	Init();
}

std::string CarrotPlant::getHoverInfo()
{
	stringstream ss;
	
	ss << "Plant de carottes" << '\n';

	return ss.str();
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

Player::Player(vec2 pos, vec2 size, std::vector<std::string> const & saved_data) :
	Entity(PLAYER, pos, size, saved_data)
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
	if (window_active) {
		if (sf::Keyboard::isKeyPressed(controls->get("Haut"))) movement.y = -1;
		else if (sf::Keyboard::isKeyPressed(controls->get("Bas"))) movement.y = 1;
		else movement.y = 0;

		if (sf::Keyboard::isKeyPressed(controls->get("Gauche"))) movement.x = -1;
		else if (sf::Keyboard::isKeyPressed(controls->get("Droite"))) movement.x = 1;
		else movement.x = 0;
	}
	else {
		movement = vec2(0, 0);
	}

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
		if (e->getSolid() && e->getId() != entity_move_id) {
			auto ebox = e->getCollisionBox();
			vec2 epos = {ebox.left, ebox.top};
			vec2 esize ={ebox.width, ebox.height};

			float tolerance = 10;

			vec2 d(8, 8);
			auto p = pos + d;
			auto s = size - d;

			if (movement.x != 0 && p.y + s.y > epos.y && p.y < epos.y + esize.y) {
				if (p.x + s.x >= epos.x && p.x + s.x <= epos.x + tolerance
				 && p.x <= epos.x && movement.x > 0) {
					pos.x = epos.x - s.x - d.x;
					movement.x = 0;
				}
				else if (p.x <= epos.x + esize.x && p.x >= epos.x + esize.x - tolerance
					  && p.x + s.x >= epos.x + esize.x && movement.x < 0) {
					pos.x = epos.x + esize.x - d.x;
					movement.x = 0;
				}
			}

			if (movement.y != 0 && p.x + s.x > epos.x && p.x < epos.x + esize.x) {
				if (p.y + s.y >= epos.y && p.y + s.y <= epos.y + tolerance
				 && p.y <= epos.y && movement.y > 0) {
					pos.y = epos.y - s.y - d.y;
					movement.y = 0;
				}
				else if (p.y <= epos.y + esize.y && p.y >= epos.y + esize.y - tolerance
					  && p.y + s.y >= epos.y + esize.y && movement.y < 0) {
					pos.y = epos.y + esize.y - d.y;
					movement.y = 0;
				}
			}

		}
	}

	if (pos.x + size.x > WORLD_W) {
		pos.x = WORLD_W - size.x;
		movement.x = 0;
	}
	else if (pos.x < 0) {
		pos.x = 0;
		movement.x = 0;
	}
	if (pos.y + size.y > WORLD_H) {
		pos.y = WORLD_H - size.y;
		movement.y = 0;
	}
	else if (pos.y < 0) {
		pos.y = 0;
		movement.y = 0;
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

std::string Player::getHoverInfo()
{
	return std::string("Joueur\n");
}
