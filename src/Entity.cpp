#include "Entity.h"
#include "ResourceManager.h"

#include "Globals.h"

#include <sstream>
#include <iostream>
using namespace std;

int Entity::last_id = 0;

std::vector<std::string> Entity::getSavedData()
{
	//cout << "Warning: Entity::getSavedData() called." << endl;
	return std::vector<std::string>();
}

std::string Entity::getHoverInfo()
{
	//cout << "Warning: Entity::getHoverInfo() called." << endl;
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

	root_pos = pos;

	if (saved_data.size() != 0) {
		pos_adjusted = true;
		type = getEntityTypeFromString(saved_data[0]);
		growth_level = atoi(saved_data[1].c_str());
		variation = growth_level;
		hp = atoi(saved_data[2].c_str());
		fruits = min(4, atoi(saved_data[3].c_str()));
		root_pos.x = float(atoi(saved_data[4].c_str()));
		root_pos.y = float(atoi(saved_data[5].c_str()));
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
		pos = root_pos - vec2(size.x / 2.f, size.y);
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

void TreeObj::GrowOneLevel()
{
	setGrowthLevel(min(6, getGrowthLevel() + 1));
}

void TreeObj::setGrowthLevel(int level)
{
	growth_level = level;
	pos_adjusted = false;

	if (growth_level == 1 || growth_level == 2) {
		hp = 1;
	}
	else if (growth_level == 3) {
		hp = 2;
	}
	else if (growth_level == 4 || growth_level == 5 || growth_level == 6) {
		hp = 3;
	}

	if (growth_level == 6 && (true || fruits == -1)) {
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

void TreeObj::setPos(vec2 pos)
{
	GameObject::setPos(pos);
#ifdef EDITOR_MODE
	root_pos = pos + vec2(size.x / 2.f, size.y);
#endif
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

void CarrotPlant::GrowOneLevel()
{
	setGrowthLevel(growth_level + 50);
}

void CarrotPlant::setGrowthLevel(float growth_level)
{
	if (growth_level != -111) {
		this->growth_level = growth_level;
	}
	Init();
}

std::string CarrotPlant::getHoverInfo()
{
	stringstream ss;
	
	ss << "Plant de carottes" << '\n' << "Doit être arrosé" << '\n' << "Donne 3 carottes lorsque poussé compètement" << '\n';

	return ss.str();
}

CompostBox::CompostBox(vec2 pos, const std::vector<std::string>& saved_data) : 
	GameObject(COMPOST_BOX, 0, pos, vec2(0,0), saved_data)
{
	scale = 2.f;
	solid = true;

	if (saved_data.size() != 0) {
		nb_of_bags = atoi(saved_data[0].c_str());
		bag_progress = float(atoi(saved_data[1].c_str()));
	}
}

void CompostBox::Init()
{
	UpdateCinfo();

	sprite.setTexture(ResourceManager::getTexture(cinfo.texture_name));
	sprite.setTextureRect(cinfo.texture_rect);
	sprite.setScale(scale, scale);
	size = vec2(cinfo.texture_rect.width * scale, cinfo.texture_rect.height * scale);
	sprite.setPosition(pos);
}

void CompostBox::Update(float dt)
{
	if (bag_progress >= 100) {
		bag_progress = 0;
		++nb_of_bags;
	}
}

void CompostBox::AddBioJunk()
{
	bag_progress += 20;
}

void CompostBox::TakeOneBag()
{
	nb_of_bags--;
}

void CompostBox::setOpen(bool open)
{
	if (this->open != open) {
		this->open = open;
		Init();
	}
}

void CompostBox::UpdateCinfo()
{
	int var = 1;
	if (!open) { var = 2; }
	setCoordsInfo(getCoordsInfo("compostbox" + to_string(var)));
}

std::string CompostBox::getHoverInfo()
{
	stringstream ss;

	ss << "Bac de compst" << endl;
	ss << "Nombre de sacs: " << nb_of_bags << endl;
	ss << "Progression du prochain sac: " << bag_progress << "%" << endl;
	if (nb_of_bags != 0)
		ss << "Doit être vide pour être déplacé" << endl;

	return ss.str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void AnimComp::Init()
{
	state = AnimState::IDLE;
	last_state = AnimState::WALKING;
	dir = AnimDir::DOWN;
	last_dir = AnimDir::UP;

	relative_origin = vec2(15, 61);

	texture = &ResourceManager::getTexture("Textures_Character.png");
	sprite.setTexture(*texture);
	sprite.setPosition(*entity_pos);
	sprite.setTextureRect(sf::IntRect(vec2i(0,0), vec2i(int(frame_width), int(frame_height))));
	sprite.setScale(scale, scale);

	frame_time = sf::seconds(0.1f);
}

void AnimComp::Update()
{
	sprite.setPosition(*entity_pos - relative_origin*scale);

	if (dir != last_dir || state != last_state) {
		last_dir = dir;
		last_state = state;

		frame = 0;
		if (state == AnimState::IDLE) {
			max_frame = 0;
			frame_y = 0;
			if (dir == AnimDir::UP) {
				frame_x = 2;
			}
			else if (dir == AnimDir::DOWN) {
				frame_x = 1;
			}
			else if (dir == AnimDir::LEFT) {
				frame_x = 0;
			}
			else if (dir == AnimDir::RIGHT) {
				frame_x = 0;
			}
		}
		else if (state == AnimState::WALKING) {
			frame_x = 0;
			if (dir == AnimDir::UP) {
				max_frame = 5;
				frame_y = 8;
			}
			else if (dir == AnimDir::DOWN) {
				max_frame = 5;
				frame_y = 6;
			}
			else if (dir == AnimDir::LEFT) {
				max_frame = 7;
				frame_y = 3;
			}
			else if (dir == AnimDir::RIGHT) {
				max_frame = 7;
				frame_y = 3;
			}
		}
	}

	if (state == AnimState::WALKING) {
		if (clock.getElapsedTime() >= frame_time) {
			clock.restart();
			if (frame != max_frame - 1) ++frame;
			else frame = 0;
		}
	}

	if (dir == AnimDir::LEFT) {
		sprite.setTextureRect(sf::IntRect(int((frame_x + frame) * frame_width + frame_width), int(frame_y * frame_height), int(-frame_width), int(frame_height*2)));
	}
	else {
		sprite.setTextureRect(sf::IntRect(int((frame_x + frame) * frame_width), int(frame_y * frame_height), int(frame_width), int(frame_height*2)));
	}

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

	anim.entity_pos = &pos;
	anim.Init();
}

void Player::Update(float dt)
{
	if (window_active) {
		if (sf::Keyboard::isKeyPressed(controls->get("Haut"))) {
			movement.y = -1;
			anim.state = AnimState::WALKING;
			anim.dir = AnimDir::UP;
		}
		else if (sf::Keyboard::isKeyPressed(controls->get("Bas"))) {
			movement.y = 1;
			anim.state = AnimState::WALKING;
			anim.dir = AnimDir::DOWN;
		}
		else {
			movement.y = 0;
			anim.state = AnimState::IDLE;
		}

		if (sf::Keyboard::isKeyPressed(controls->get("Gauche"))) {
			movement.x = -1;
			anim.state = AnimState::WALKING;
			anim.dir = AnimDir::LEFT;
		}
		else if (sf::Keyboard::isKeyPressed(controls->get("Droite"))) {
			movement.x = 1;
			anim.state = AnimState::WALKING;
			anim.dir = AnimDir::RIGHT;
		}
		else {
			movement.x = 0;
			anim.state = AnimState::IDLE;
		}
	}
	else {
		movement = vec2(0, 0);
		anim.state = AnimState::IDLE;
	}

	if (movement != vec2(0, 0)) {
		movement = normalize(movement);
		movement.y *= 0.92f;
		anim.state = AnimState::WALKING;
	}
	else {
		anim.state = AnimState::IDLE;
	}

	anim.Update();
}

void Player::Render(sf::RenderTarget & target)
{
	target.draw(anim.sprite);
}

void Player::DoCollisions(std::vector<Entity*>& entities, int entity_move_id)
{
	for (auto e : entities) {
		if (e->getSolid() && e->getId() != entity_move_id) {
			auto ebox = e->getCollisionBox();
			vec2 epos = {ebox.left, ebox.top};
			vec2 esize ={ebox.width, ebox.height};

			float tolerance = 10;

			auto p = vec2(getCollisionBox().left, getCollisionBox().top);
			auto s = vec2(getCollisionBox().width, getCollisionBox().height);

			if (movement.x != 0) {
				if (p.y + s.y > epos.y && p.y < epos.y + esize.y) {
					if (p.x + s.x >= epos.x && p.x + s.x <= epos.x + tolerance
						&& p.x <= epos.x && movement.x > 0) {
						pos.x = epos.x - s.x*0.5f;
						movement.x = 0;
					}
					else if (p.x <= epos.x + esize.x && p.x >= epos.x + esize.x - tolerance
						&& p.x + s.x >= epos.x + esize.x && movement.x < 0) {
						pos.x = epos.x + esize.x + s.x*0.5f;
						movement.x = 0;
					}
				}
			}

			if (movement.y != 0 ) {
				if (p.x + s.x > epos.x && p.x < epos.x + esize.x) {
					if (p.y + s.y >= epos.y && p.y + s.y <= epos.y + tolerance
					 && p.y <= epos.y && movement.y > 0) {
						pos.y = epos.y;
						movement.y = 0;
					}
					else if (p.y <= epos.y + esize.y && p.y >= epos.y + esize.y - tolerance
						  && p.y + s.y >= epos.y + esize.y && movement.y < 0) {
						pos.y = epos.y + esize.y + s.y;
						movement.y = 0;
					}
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
}

std::string Player::getHoverInfo()
{
	return std::string("Joueur\n");
}

sf::FloatRect const Player::getCollisionBox()
{
	return sf::FloatRect(pos.x - 7*anim.scale, pos.y - 13*anim.scale, (7*anim.scale)*2, 13*anim.scale);
}

