#include "World.h"

#include "GameState.h"
#include "ResourceManager.h"
#include "rng.h"
#include "Editor/Editor.h"
#include "GameGUI.h"

#include <deque>
#include <fstream>
#include <iostream>
using namespace std;

sf::Vector2f mouse_pos = {-1, -1};

World::World() :
	game_view(sf::FloatRect({0,0}, {float(WINDOW_WIDTH), float(WINDOW_HEIGHT)}))
{
	game_view.setSize(WINDOW_WIDTH*0.85f, WINDOW_HEIGHT*0.85f);
}

World::World(Controls* controls) :
	game_view(sf::FloatRect({0,0}, {float(WINDOW_WIDTH), float(WINDOW_HEIGHT)})),
	controls(controls), inventory(inventory)
{

}
World::~World()
{
	Clear();
}

void World::Init(Inventory* inventory, InventoryButton* inv_butt)
{
	this->inventory = inventory;
	this->inv_butt = inv_butt;
}

void World::Clear()
{
	for (int i = 0; i != entities.size(); ++i) {
		delete entities[i];
	}
	entities.clear();
	items.clear();
	ground.Clear();
}

void World::CreateAndSaveWorld(std::string const & filename)
{
	Clear();
	name = filename;

	const int w = 100, h = 100;
	vector<int> t(w*h, 0);
	for (int i = 0; i != w*h; ++i) {
		t[i] = 0;
	}
	ground.LoadTileMap(t, w, h);

	player = new Player();
	player->setControls(controls);
	entities.push_back(player);

	Save();
}

void World::CreateNewBlank(const std::string & filename)
{
	Clear();
	name = filename;

	player = new Player();
	player->setControls(controls);
	entities.push_back(player);

	vector<int> t(WORLD_W/visual_tile_size * WORLD_H/visual_tile_size, NONE);
	ground.LoadTileMap(t, WORLD_W/visual_tile_size, WORLD_H/visual_tile_size);

	Save();
}

void World::LoadWorld(std::string const & filename)
{
	cout << "[Load Started]" << endl;
	Clear();
	name = filename;
	std::ifstream s("Resources/Data/Saves/" + filename);

	std::string w, str;

	sf::Clock clock;

	cout << "loading entities";
	while (s >> w) {
		if (w == "e") {
			cout << ".";
			Type t;
			s >> str;
			t = Type(atoi(str.c_str()));

			sf::Vector2f p;
			s >> str;
			p.x = stof(str);
			s >> str;
			p.y = stof(str);

			sf::Vector2f sz;
			s >> str;
			sz.x = stof(str);
			s >> str;
			sz.y = stof(str);

			unsigned long f;
			s >> str;
			f = atol(str.c_str()); // Might need to be unsigned

			std::vector<std::string> vec;
			str = "";
			bool b = false;
			char c = 0;
			while (s.get(c) && c != '\n') {
				if (c == '"') {
					if (!b) b = true;
					else {
						b = false;
						vec.push_back(str);
						str = "";
					}
				}
				else if (b) {
					str += c;
				}
			}

			Player* pl;
			GameObject* go;
			ItemObject* io;
			int id;

			switch (t)
			{
			case ENTITY:
				cerr << "ERROR in save file: \"Data/Saves/" << filename << "\": entity saved as type Entity, which is a pure virtual class." << endl;
				break;
			case PLAYER:
				pl = new Player(p, sz, f, vec);
				player = pl;
				entities.push_back(player);
				break;
			case GAME_OBJECT:
				break;
			case ROCK:
				go = make_rock(p);
				entities.push_back(go);
				break;
			case BUSH:
				go = make_bush(p);
				entities.push_back(go);
				break;
			case TREE:
				go = make_tree(p);
				entities.push_back(go);
				break;
			case ITEM:
				id = Item::Manager::CreateItem(Item::getItemTypeByName(vec[0]));
				io = make_item(id, p);
				entities.push_back(io);
				items.push_back(io);
				break;
			default:
				break;
			}
		}
		else if (w == "[TILE_MAP]") {
			cout << "   done! (" << clock.restart().asSeconds() << " s)" << endl << "loading tile_map...";
			vector<int> tiles;
			int width, height;
			s >> width;
			s >> height;
			while (s >> w) {
				if (w == "[END]") break;
				tiles.push_back(stoi(w));
			}

			ground.LoadTileMap(tiles, width, height);
		}
		else if (w == "[INVENTORY]") {
			cout << "   done! (" << clock.restart().asSeconds() << " s)" << endl << "loading inventory...";
			while (s >> w) {
				if (w == "[END]") break;
				
				if (w[0] == '"') { // remove ""
					w = w.substr(1, w.size());
					if (w[w.size()-1] == '"') {
						w = w.substr(0, w.size()-1);
					}
					else {
						char c;
						while (s.get(c)) {
							if (c == '"') break;
							w += c;
						}
					}
				}
				inventory->AddNewItem(Item::getItemTypeByName(w));
			}
			cout << "   done! (" << clock.restart().asSeconds() << " s)" << endl;
		}
	}
	player->setControls(controls);
	cout << "[Load Ended]" << endl;
}

void World::Save(const string& filename)
{
	if (filename != "") {
		name = filename;
	}
	bool file_existed = false;
	std::ifstream f("Resources/Data/Saves/" + name, std::ios::binary);
	if (f.is_open()) file_existed = true;

	if (file_existed) {
		std::ofstream backup("Resources/Data/Saves/" + name + ".backup", std::ios::binary);
		backup << f.rdbuf();
		backup.close();
	}

	f.close();

	std::ofstream s("Resources/Data/Saves/" + name);

	sf::Clock clock;
	cout << "saving entities...";
	for (auto e : entities) {
		s << "e " << e->getType() << " "
			<< e->getPos().x << " " << e->getPos().y << " "
			<< e->getSize().x << " " << e->getSize().y << " "
			<< e->getFlags() << " ";
		for (auto & str : e->getSavedData()) { s << '"' << str << "\" "; }
		s << endl;
	}
	cout << " - done! (" << clock.restart().asSeconds() << " s)" << endl;

	cout << "saving tilemap...";
	s << "[TILE_MAP]" << endl;
	s << ground.getWidth() << endl << ground.getHeight() << endl;
	for (auto t : ground.getTiles()) {
		s << t.getType() << " ";
	}
	s << endl << "[END]" << endl;
	cout << " - done! (" << clock.restart().asSeconds() << " s)" << endl;

	cout << "saving inventory...";
	s << "[INVENTORY]" << endl;
	auto items = inventory->getItemsId();
	for (auto i : items) {
		auto item = Item::Manager::getAny(i);
		s << "\"" << item->name << "\"" << endl;
	}
	s << "[END]" << endl;
	cout << " - done! (" << clock.restart().asSeconds() << " s)" << endl;

	if (!file_existed) {
		std::ofstream stream("Resources/Data/Saves/all_saves", std::ios_base::app);
		stream << name << endl;
	}

	cout << "World saved to \"Resources/Data/Saves/" << name << "\"" << endl;
}

void World::Update(float dt, sf::Vector2f mouse_pos_in_world)
{
	auto mpw = mouse_pos_in_world;

	entity_hovered = nullptr;

	for (auto i = entities.begin(); i != entities.end();) {
		auto e = *i;
		if (!e->getDead()) {
			e->Update(dt);
			
			if (!item_move && !item_place) {
				if (mpw.x >= e->getPos().x && mpw.x <= e->getPos().x + e->getSize().x &&
					mpw.y >= e->getPos().y && mpw.y <= e->getPos().y + e->getSize().y) {
					entity_hovered = e;
				}
			}
		}
		else {
			if (e->getType() == ITEM) {
				DeleteItemObj(e->getId());
			}
			i = entities.erase(i);
			continue;
		}
		++i;
	}

	int no_collision_id = -1; // this entity won't have collision (because it is picked up)
	if (item_place) no_collision_id = item_place->getId();
	if (item_move) no_collision_id = item_move->getId();

	player->DoCollisions(entities, no_collision_id);
	player->DoMovement(dt);

	if (item_place) item_place->setPos(mouse_pos_in_world - item_place->getSize()/2.f + item_place->getOrigin());
	if (item_move) item_move->setPos(mouse_pos_in_world - item_move->getSize()/2.f + item_move->getOrigin());

	UpdateView();
}

void World::UpdateView()
{

	float vx = float(int(player->getPos().x + player->getSize().x/2.f));
	float vy = float(int(player->getPos().y + player->getSize().y/2.f));
	auto vs = game_view.getSize();

	if (vx - vs.x/2.f <= 0)			vx = vs.x/2.f;
	if (vx + vs.x/2.f >= WORLD_W)	vx = WORLD_W - vs.x/2.f;
	if (vy - vs.y/2.f <= 0)			vy = vs.y/2.f;
	if (vy + vs.y/2.f >= WORLD_H)	vy = WORLD_H - vs.y/2.f;
	
	game_view.setCenter(vx, vy);
}

void World::Render(sf::RenderTarget & target)
{
#ifndef EDITOR_MODE
	target.setView(game_view);
#endif

	target.draw(ground);

	vector<Entity*> entities_front;

	if (player) {
		for (auto e : entities) {
			if (e->getType() != PLAYER) {
				if (player->getPos().y + player->getSize().y > e->getPos().y + e->getSize().y) {
					e->Render(target);
				}
				else {
					entities_front.push_back(e);
				}
			}
		}

		player->Render(target);

		for (auto e : entities_front) {
			if (e->getType() != PLAYER) {
				e->Render(target);
			}
		}

		if (item_place) item_place->Render(target);
	}
	else {
		for (auto e : entities) {
			e->Render(target);
		}
	}

	//target.setView(target.getDefaultView());
}

sf::Vector2i drag_mouse_pos;
bool middle_pressed = false;

bool World::HandleEvent(sf::Event const & event)
{
	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::F6) {
			Save();
		}
	}
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Button::Left) {
			if (item_place) {
				if (inv_butt && inv_butt->getOpen()) {
					inventory->AddItem(item_place->getItemId());
				}
				else {
					entities.push_back(item_place);
					items.push_back(item_place);
				}
				item_place = nullptr;
			}
			else if (entity_hovered) {
				if (entity_hovered->getType() == ITEM) {
					item_move = FindItem(entity_hovered->getId());
				}
				entity_hovered = nullptr;
			}
		}
	}
	if (event.type == sf::Event::MouseButtonReleased) {
		if (item_move) {
			if (inv_butt && inv_butt->getOpen()) {
				inventory->AddItem(item_move->getItemId());
				DeleteEntity(item_move->getId());
			}
			item_move = nullptr;
		}
	}
	if (event.type == sf::Event::MouseMoved) {
	}


	return false;
}

Entity * World::FindEntityClicked(sf::Vector2f mpos)
{
	for (auto e : entities) {
		auto ep = e->getPos();
		auto es = e->getSize();

		if (mpos.x > ep.x && mpos.x < ep.x + es.x) {
			if (mpos.y > ep.y && mpos.y < ep.y + es.y) {
				return e;
			}
		}
	}
	return nullptr;
}

Entity* World::getEntity(int id)
{
	int index = -1;
	for (uint i = 0; i != entities.size(); ++i) {
		if (entities[i]->getId() == id) {
			index = i;
			break;
		}
	}
	if (index != -1) return entities[index];
	return nullptr;
}

ItemObject * World::FindItem(int id)
{
	for (auto i : items) {
		if (i->getId() == id) {
			return i;
		}
	}
	return nullptr;
}

void World::DuplicateEntity(int id)
{
	auto e = getEntity(id);
	if (e) {
		auto type = e->getType();
		auto p = e->getPos();
		
		AddEntity(make_entity(type, p));
	}
}

void World::DeleteEntity(int id)
{
	int index = -1;
	for (uint i = 0; i != entities.size(); ++i) {
		if (entities[i]->getId() == id) {
			index = i;
			break;
		}
	}
	if (index != -1) {
		delete entities[index];
		entities.erase(entities.begin() + index);
	}
}

void World::DeleteItemObj(int id)
{
	int index = -1;
	for (uint i = 0; i != items.size(); ++i) {
		if (items[i]->getId() == id) {
			index = i;
			break;
		}
	}
	if (index != -1) { items.erase(items.begin() + index); }
	else { cerr << "Could not find item " << id << endl; }

}

void World::StartPlaceItem(ItemObject* item) { item_place = item; }
