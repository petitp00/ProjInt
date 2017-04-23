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

vec2 mouse_pos = {-1, -1};
vec2 mouse_pos_in_world(-1, -1);

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

void World::Init(Inventory* inventory, InventoryButton* inv_butt, GameState* game_state)
{
	this->inventory = inventory;
	this->inv_butt = inv_butt;
	this->game_state= game_state;

	particle_manager.Init(this);
}

void World::Clear()
{
	for (int i = 0; i != entities.size(); ++i) {
		delete entities[i];
	}
	entities.clear();
	items.clear();
	trees.clear();
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
	std::map<int, int> id_map; // maps old ids (saved in the file) to new ids
	int old_equipped_id = -1;
	sf::Clock clock;

	cout << "loading items & entities";

	while (s >> w) {
		if (w == "equipped_tool") {
			s >> w;
			old_equipped_id = stoi(w);
		}
		else if (w == "i") {
			cout << ".";
			int id;
			s >> str;
			id = stoi(str);

			string iname;
			s >> iname;
			
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
			int new_id = Item::Manager::CreateItem(Item::getItemTypeByName(iname), vec);
			id_map[id] = new_id;
		}
		else if (w == "e") {
			cout << ".";
			Type t;
			s >> str;
			t = Type(atoi(str.c_str()));

			vec2 p;
			s >> str;
			p.x = stof(str);
			s >> str;
			p.y = stof(str);

			vec2 sz;
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
			TreeObj* to;
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
				go = make_rock(p, stoi(vec[0]));
				entities.push_back(go);
				break;
			case ITEM:
				id = Item::Manager::CreateItem(Item::getItemTypeByName(vec[0]));
				io = make_item(id, p);
				entities.push_back(io);
				items.push_back(io);
				break;
			case APPLE_TREE:
				to = make_tree_obj(APPLE_TREE, stoi(vec[1]), p, vec);
				entities.push_back(to);
				trees.push_back(to);
				break;
			case BANANA_TREE:
				to = make_tree_obj(BANANA_TREE, stoi(vec[1]), p, vec);
				entities.push_back(to);
				trees.push_back(to);
				break;
			case HUT:
				go = make_hut(p);
				entities.push_back(go);
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
				int id = stoi(w);
				inventory->AddItem(id_map[id]);
			}
			cout << "   done! (" << clock.restart().asSeconds() << " s)" << endl;
		}
	}
#ifndef EDITOR_MODE
	player->setControls(controls);
	if (old_equipped_id != -1 && id_map.count(old_equipped_id)) {
		game_state->EquipTool(id_map[old_equipped_id]);
	}
#endif
	SortEntities();
	cout << "[Load Ended]" << endl;
}

void World::Save(const string& filename)
{
	if (filename != "") name = filename;
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

	cout << "saving items...";
#ifndef EDITOR_MODE
	s << "equipped_tool " << game_state->getEquippedTool() << endl;
#endif
#ifdef EDITOR_MODE
	s << "equipped_tool " << -1 << endl;
#endif

	for (int id : inventory->getItemsId()) {
		auto ia = Item::Manager::getAny(id);
		s << "i " << id << ' ' << ia->name << ' ';
		for (auto& sv : ia->getSaveData()) { s << '"' << sv << "\" "; }
		s << endl;
	}
	cout << " - done! (" << clock.restart().asSeconds() << " s)" << endl;

	cout << "saving entities...";
	for (auto e : entities) {
		s << "e "	<< e->getType()		<< ' '
					<< e->getPos().x	<< ' ' << e->getPos().y		<< ' '
					<< e->getSize().x	<< ' ' << e->getSize().y	<< ' '
					<< e->getFlags()	<< ' ';

		for (auto & str : e->getSavedData()) { s << '"' << str << "\" "; }
		s << endl;
	}
	cout << " - done! (" << clock.restart().asSeconds() << " s)" << endl;

	cout << "saving tilemap...";
	s << "[TILE_MAP]" << endl;
	s << ground.getWidth() << endl << ground.getHeight() << endl;
	for (auto t : ground.getTiles()) { s << t.getType() << ' '; }
	s << endl << "[END]" << endl;
	cout << " - done! (" << clock.restart().asSeconds() << " s)" << endl;

	cout << "saving inventory...";
	s << "[INVENTORY]" << endl;
	for (auto i : inventory->getItemsId()) { s << i << endl; }
	s << "[END]" << endl;
	cout << " - done! (" << clock.restart().asSeconds() << " s)" << endl;

	if (!file_existed) {
		std::ofstream stream("Resources/Data/Saves/all_saves", std::ios_base::app);
		stream << name << endl;
	}

	cout << "World saved to \"Resources/Data/Saves/" << name << "\"" << endl;
}

void World::Update(float dt, vec2 mouse_pos_in_world)
{
	::mouse_pos_in_world = mouse_pos_in_world;
	auto mpw = mouse_pos_in_world;

	entity_hovered.clear();

	for (auto i = entities.begin(); i != entities.end();) {
		auto e = *i;
		if (!e->getDead()) {
			e->Update(dt);
			
			if (!item_move && !item_place) {
				auto ec = e->getCollisionBox();
				if (mpw.x >= e->getPos().x && mpw.x <= e->getPos().x + e->getSize().x &&
					mpw.y >= e->getPos().y && mpw.y <= e->getPos().y + e->getSize().y) {
					entity_hovered.push_back(e);
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

	particle_manager.UpdateParticles(dt);

	int no_collision_id = -1; // this entity won't have collision (because it is picked up)
	if (item_place) no_collision_id = item_place->getId();
	if (item_move) no_collision_id = item_move->getId();

	player->DoCollisions(entities, no_collision_id);
	player->DoMovement(dt);

	if (item_place) item_place->setPos(mpw - item_place->getSize()/2.f + item_place->getOrigin());
	if (item_move) item_move->setPos(mpw - item_move->getSize()/2.f + item_move->getOrigin());

	UpdateView();

	if (true || entities_need_sorting) {
		SortEntitiesImpl();
		entities_need_sorting = false;
	}
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
	if (player) {
		int next_sprite_particle_to_render = 0;
		int next_item_particle_to_render = 0;
		for (auto e : entities) {
			if (next_sprite_particle_to_render != -1)
				next_sprite_particle_to_render = particle_manager.RenderSpriteParticlesLowerThan(target, e->getPos().y + e->getSize().y, next_sprite_particle_to_render);
			if (next_item_particle_to_render != -1)
				next_item_particle_to_render = particle_manager.RenderItemParticlesLowerThan(target, e->getPos().y + e->getSize().y, next_item_particle_to_render);
			e->Render(target);
		}

		// render the remaining particles
		if (next_sprite_particle_to_render != -1)
			particle_manager.RenderSpriteParticlesLowerThan(target, 100000, next_sprite_particle_to_render);
		if (next_item_particle_to_render != -1)
			particle_manager.RenderItemParticlesLowerThan(target, 100000, next_item_particle_to_render);

		if (item_place) item_place->Render(target);
	}
	else {
		for (auto e : entities) {
			e->Render(target);
		}
	}
}

vec2i drag_mouse_pos;
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
			else if (entity_hovered.size() != 0) {
				for (auto e : entity_hovered) {
					if (e->getType() == ITEM) {
						item_move = FindItem(e->getId());
						entity_hovered.clear();
						break;
					}
				}
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

Entity * World::FindEntityClicked(vec2 mpos)
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

bool World::getCanUseTool(int tool)
{
	if (tool) {
		auto t = Item::Manager::getTool(tool);
		string name = t->name;

		if (name == "Bol") {
			auto b = Item::Manager::getBowl(tool);
			GroundType gtype = ground.getTileClicked(mouse_pos_in_world);

			if (gtype == GroundType::RIVER) {
				if (b->water_level < 4) {
					return true;
				}
			}

			if (b->water_level > 0) {
				auto pp = player->getPos();
				auto ps = player->getSize();
				auto m = mouse_pos_in_world;
				if (m.x > pp.x && m.x < pp.x + ps.x && m.y > pp.y && m.y < pp.y + ps.y) {
					return true;
				}
			}

		}
		if (entity_hovered.size() != 0) {
			for (auto e : entity_hovered) {
				auto ent_type = e->getType();
				if (name == "Hache") {
					if (ent_type == APPLE_TREE || ent_type == BANANA_TREE) return true;
				}
			}
		}
	}

	return false;
}

bool World::getCanCollect(Item::ItemType& item_type)
{
	for (auto t : trees) {
		auto tp = t->getPos();
		auto ts = t->getSize();
		auto m = mouse_pos_in_world;

		if (m.x > tp.x && m.x < tp.x + ts.x && m.y > tp.y && m.y < tp.y + ts.y) {
			if (t->getGrowthLevel() == 6) {
				if (t->getType() == APPLE_TREE)
					item_type = Item::ItemType::apple;
				else if (t->getType() == BANANA_TREE)
					item_type = Item::ItemType::banana;
				
				return true;
			}
		}
	}
	return false;
}

void World::Collect()
{
	auto m = mouse_pos_in_world;

	for (auto t : trees) {
		auto tp = t->getPos();
		auto ts = t->getSize();

		if (m.x > tp.x && m.x < tp.x + ts.x && m.y > tp.y && m.y < tp.y + ts.y) {
			if (t->getGrowthLevel() == 6) {
				int leaf_amount = rng::rand_int(3, 8);
				for (int i = 0; i != leaf_amount; ++ i) {
					vec2 pos;
					pos.x = rng::rand_float(tp.x + ts.x/6.f, tp.x+ts.x - ts.x/6.f);
					pos.y = rng::rand_float(tp.y + ts.y/6.f, tp.y+ts.y/3.f*2.f);
					float end_y = pos.y + ts.y/2.f;
					particle_manager.CreateSpriteParticle(Particle::SpriteParticleType::Leaf, pos, end_y);
				}

				if (t->getFruitsAmount()) {
					Item::ItemType type;
					if (t->getType() == APPLE_TREE) {
						type = Item::ItemType::apple;
					}
					else if (t->getType() == BANANA_TREE) {
						type = Item::ItemType::banana;
					}
					vec2 pos;
					pos.x = rng::rand_float(tp.x + ts.x/6.f, tp.x+ts.x - ts.x/6.f);
					pos.y = rng::rand_float(tp.y + ts.y/6.f, tp.y+ts.y/3.f*2.f);
					float end_y = pos.y + ts.y/2.f;
					particle_manager.CreateItemParticle(type, pos, end_y);

					t->TakeOneFruit();
				}
				particle_manager.SortSpriteParticles();
				particle_manager.SortItemParticles();
				break;
			}
		}

		
	}
}

void World::UseEquippedToolAt()
{
	auto m = mouse_pos_in_world;
	int t = game_state->getEquippedTool();
	if (t != -1) {
		auto tool = Item::Manager::getTool(t);

		if (tool->name == "Bol") {
			auto b = Item::Manager::getBowl(t);
			GroundType gtype = ground.getTileClicked(mouse_pos_in_world);

			if (gtype == GroundType::RIVER) {
				if (b->water_level < 4) {
					b->water_level = 4;
				}
			}

			if (b->water_level > 0) {
				auto pp = player->getPos();
				auto ps = player->getSize();
				if (m.x > pp.x && m.x < pp.x + ps.x && m.y > pp.y && m.y < pp.y + ps.y) {
					--b->water_level;
					// DRINKING


				}
			}

			b->UpdatePosInTextureMap();
		}

		// TREE CUTTING
		if (tool->name == "Hache") {
			for (auto tree : trees) {
				auto tp = tree->getPos();
				auto ts = tree->getSize();
				
				if (m.x > tp.x && m.x < tp.x + ts.x && m.y > tp.y && m.y < tp.y + ts.y) {
					tree->Hit();

					// Create leaf particles
					int leaf_amount = rng::rand_int(7, 14);
					for (int i = 0; i != leaf_amount; ++ i) {
						vec2 pos;
						pos.x = rng::rand_float(tp.x + ts.x/6.f, tp.x+ts.x - ts.x/6.f);
						pos.y = rng::rand_float(tp.y + ts.y/6.f, tp.y+ts.y/3.f*2.f);
						float end_y = pos.y + ts.y/2.f;
						particle_manager.CreateSpriteParticle(Particle::SpriteParticleType::Leaf, pos, end_y);
					}

					// Create wood particles
					int wood_amount = rng::rand_int(3, 6);
					Particle::SpriteParticleType wood_part_type;
					if (tree->getType() == APPLE_TREE) wood_part_type = Particle::SpriteParticleType::AppleWood;
					else if (tree->getType() == BANANA_TREE) wood_part_type = Particle::SpriteParticleType::BananaWood;
					for (int i = 0; i != wood_amount; ++i) {
						vec2 pos;
						pos.x = rng::rand_float(tp.x + ts.x/3.f, tp.x+ts.x - ts.x/3.f);
						pos.y = rng::rand_float(tp.y + ts.y/2.f, tp.y+ts.y/4.f*3.5f);
						float end_y = pos.y + ts.y/2.f;
						particle_manager.CreateSpriteParticle(wood_part_type, pos, end_y);
					}

					if (tree->getChopped()) {
						auto items_dropped = tree->getDroppedItems();
						for (auto i : items_dropped) {
							for (int n = 0; n != i.second; ++n) {
								vec2 pos;
								pos.x = rng::rand_float(tp.x + ts.x/3.f, tp.x+ts.x - ts.x/3.f);
								pos.y = rng::rand_float(tp.y + ts.y/2.f, tp.y+ts.y/4.f*3.5f);
								float end_y = pos.y + ts.y/2.f;
								particle_manager.CreateItemParticle(i.first, pos, end_y);
							}
						}
						DeleteTree(tree->getId());
					}

					particle_manager.SortSpriteParticles();
					particle_manager.SortItemParticles();
				}
			}
		}
	}
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

void World::DeleteTree(int id)
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

	index = -1;
	for (uint i = 0; i != trees.size(); ++i) {
		if (trees[i]->getId() == id) {
			index = i;
			break;
		}
	}

	if (index != -1) {
		trees.erase(trees.begin() + index);
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

void World::SortEntitiesImpl() // seems to be super fast for not a lot of elements (~10µs)
{
	sort(entities.begin(), entities.end(), [](auto e1, auto e2) {
		return e1->getPos().y + e1->getSize().y < e2->getPos().y + e2->getSize().y;
	});
}
