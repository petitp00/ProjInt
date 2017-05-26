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
	float scale = 0.75f;
	game_view.setSize(WINDOW_WIDTH*scale, WINDOW_HEIGHT*scale);
}

World::World(Controls* controls) :
	game_view(sf::FloatRect({0,0}, {float(WINDOW_WIDTH), float(WINDOW_HEIGHT)})),
	controls(controls), inventory(inventory)
{
	float scale = 0.75100f;
	vec2 viewsize = vec2(float(WINDOW_WIDTH), float(WINDOW_HEIGHT));
	game_view.reset(sf::FloatRect({0,0}, viewsize));
	game_view.setSize(viewsize * scale);
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
	carrot_plants.clear();
	compost_boxes.clear();
	ground.Clear();

	inventory->Clear();
}

void World::CreateAndSaveWorld(std::string const & filename)
{
	CreateNewBlank(filename);
}

void World::CreateNewBlank(const std::string & filename)
{
	Clear();
	name = filename;

	player = new Player();
	player->setControls(controls);
	entities.push_back(player);

	vector<vector<int>> t(WORLD_W / visual_tile_size * WORLD_H / visual_tile_size, { NONE, 0,0,0});
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

	auto get_string = [&](bool expect_first_quote = true) {
		str = "";
		bool b = !expect_first_quote;
		char c = 0;
		while (s.get(c) && c != '\n') {
			if (c == '"') {
				if (!b) b = true;
				else {
					return str;
				}
			}
			else if (b) {
				str += c;
			}
		}
		return string("get_string lambda failed");
	};

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

			string iname = get_string();
			
			std::vector<std::string> vec;
			char c = 0;
			while (s.get(c) && c != '\n') {
				if (c == '"') {
					vec.push_back(get_string(false));
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

			std::vector<std::string> vec;
			char c = 0;
			while (s.get(c) && c != '\n') {
				if (c == '"') {
					vec.push_back(get_string(false));
				}
			}


			Player* pl;
			GameObject* go;
			ItemObject* io;
			TreeObj* to;
			CompostBox* cb;
			int id;

			switch (t)
			{
			case ENTITY:
				cerr << "ERROR in save file: \"Data/Saves/" << filename << "\": entity saved as type Entity, which is a pure virtual class." << endl;
				break;
			case PLAYER:
				pl = new Player(p, sz, vec);
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
			case COMPOST_BOX:
				cb = make_compost_box(p, vec);
				AddCompostBox(cb);
				break;
			default:
				break;
			}
		}
		else if (w == "[TILE_MAP]") {
			cout << "   done! (" << clock.restart().asSeconds() << " s)" << endl << "loading tile_map...";
			vector<vector<int>> tiles;
			vector<int> tinfo;
			int width, height;
			s >> width;
			s >> height;
			while (s >> w) {
				if (w == "[END]") break;
				if (w == ",") { //////////////////////////// DONT FORGET TO PUT SPACES ON BOTH SIDES
					tiles.push_back(tinfo);
					tinfo.clear();
				}
				else {
					tinfo.push_back(stoi(w));
				}
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
		s << "i " << id << " \"" << ia->name << "\" ";
		for (auto& sv : ia->getSaveData()) { s << '"' << sv << "\" "; }
		s << endl;
	}
	cout << " - done! (" << clock.restart().asSeconds() << " s)" << endl;

	cout << "saving entities...";
	for (auto e : entities) {
		s << "e " << e->getType() << ' '
			<< e->getPos().x << ' ' << e->getPos().y << ' '
			<< e->getSize().x << ' ' << e->getSize().y << ' ';

		for (auto & str : e->getSavedData()) { s << '"' << str << "\" "; }
		s << endl;
	}
	cout << " - done! (" << clock.restart().asSeconds() << " s)" << endl;

	cout << "saving tilemap...";
	s << "[TILE_MAP]" << endl;
	s << ground.getWidth() << endl << ground.getHeight() << endl;
	//for (auto t : ground.getTiles()) { s << t.getType() << ' '; }
	s << ground.getSaveString();
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

	bool iplace_biojunk = false;
	if (item_place) {
		iplace_biojunk = Item::IsBioJunk(Item::Manager::getItemType(item_place->getItemId()));
	}
	bool imove_biojunk = false;
	if (item_move) {
		imove_biojunk = Item::IsBioJunk(Item::Manager::getItemType(item_move->getItemId()));
	}

	for (auto i = entities.begin(); i != entities.end();) {
		auto e = *i;
		if (!e->getDead()) {
			e->Update(dt);
			
			if (!item_move && !item_place && !compost_box_move && !compost_box_place ||
				(item_place && iplace_biojunk) ||
				(item_move && imove_biojunk)) {

				auto ec = e->getCollisionBox();
				if (mpw.x >= e->getPos().x && mpw.x <= e->getPos().x + e->getSize().x &&
					mpw.y >= e->getPos().y && mpw.y <= e->getPos().y + e->getSize().y) {
					entity_hovered.push_back(e);

					if (e->getType() == COMPOST_BOX && 
						((item_place && iplace_biojunk) || 
						(item_move && imove_biojunk))) {

						auto cb = FindCompostBox(e->getId());
						cb->setOpen(true);
						check_compost_boxes = true;
					}
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

	if (check_compost_boxes) {
		bool found_one = false; // if we found an hovered compost box
		for (auto cb : compost_boxes) {
			bool close = true; // if we need to close it
			for (auto e : entity_hovered) {
				if (e->getId() == cb->getId()) {
					found_one = true;
					close = false;
					break;
				}
			}
			if (close) {
				cb->setOpen(false);
			}
		}

		if (!found_one) {
			check_compost_boxes = false;
		}
	}

	particle_manager.UpdateParticles(dt);
	ground.UpdateRipples(dt);

	int no_collision_id = -1; // this entity won't have collision (because it is picked up)
	if (item_place) no_collision_id = item_place->getId();
	if (item_move) no_collision_id = item_move->getId();
	if (compost_box_move) no_collision_id = compost_box_move->getId();
	if (compost_box_place) no_collision_id = compost_box_place->getId();

	player->DoCollisions(entities, no_collision_id);
	player->DoMovement(dt);

	if (item_place) item_place->setPos(mpw - item_place->getSize()/2.f);
	if (item_move) item_move->setPos(mpw - item_move->getSize()/2.f);
	if (compost_box_move) compost_box_move->setPos(mpw - compost_box_move->getSize() / 2.f);
	if (compost_box_place) compost_box_place->setPos(mpw - compost_box_place->getSize() / 2.f);
	if (item_plant) {
		auto gtile = ground.getTileClicked(mpw);
		auto gtype = gtile->getType();

		if (gtype == GroundType::DIRT) {
			can_plant = true;
			item_plant->setPos(gtile->getPos()*float(visual_tile_size) + vec2(visual_tile_size / 2.f, visual_tile_size / 2.f) - item_plant->getSize() / 2.f);
		}
		else {
			can_plant = false;
		}
	}

	if (fishing) {
		fishing_shape.setStart(player->getPos() + vec2(player->getSize().x / 2.f, 0));

		if (fishing_shape.getShouldSnap()) {
			fishing = false;
			game_state->setActionInfo(ActionInfo::none);
		}
	}

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
	
	game_view.setCenter(int(vx), int(vy));
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

		if (fishing) {
			//target.draw(fish_line.rect);
			target.draw(fishing_shape);
		}

		if (item_place) item_place->Render(target);
		if (item_plant && can_plant) {
			item_plant->Render(target);
		}
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
		else if (event.key.code == sf::Keyboard::F5) {
			ground.ReloadShader();
		}
	}
	if (event.type == sf::Event::MouseButtonPressed) {
		if (fishing) {
			fishing = false;
			game_state->setActionInfo(ActionInfo::none);
		}

		if (event.mouseButton.button == sf::Mouse::Button::Left) {
			ground.StartRipple(mouse_pos_in_world, sf::seconds(1.5f), 10.f);
			if (item_place) {
				bool put_in_compost_box = false;

				if (inv_butt && inv_butt->getOpen()) {
					inventory->AddItem(item_place->getItemId());
				}
				else if (check_compost_boxes) {
					for (auto e : entity_hovered) {
						if (e->getType() == COMPOST_BOX) {
							auto cb = FindCompostBox(e->getId());
							cb->AddBioJunk();
							DeleteEntity(item_place->getId());
							put_in_compost_box = true;
							break;
						}
					}
				}
				if (!(inv_butt && inv_butt->getOpen()) && !put_in_compost_box) {
					entities.push_back(item_place);
					items.push_back(item_place);
				}
				item_place = nullptr;
				game_state->setActionInfo(ActionInfo::none);
			}
			else if (compost_box_place) {
				compost_box_place = nullptr;
				game_state->setActionInfo(ActionInfo::none);
			}
			else if (item_plant && can_plant) {
				PlantSeed();
				Item::Manager::DeleteItem(item_plant->getItemId());
				delete item_plant;
				item_plant = nullptr;
				game_state->setActionInfo(ActionInfo::none);
			}
			else if (entity_hovered.size() != 0) {
				for (auto e : entity_hovered) {
					if (e->getType() == ITEM) {
						item_move = FindItem(e->getId());
						entity_hovered.clear();
						break;
					}
					else if (e->getType() == COMPOST_BOX) {
						compost_box_move = FindCompostBox(e->getId());
						if (compost_box_move->getNbOfBags() != 0) compost_box_move = nullptr;
						entity_hovered.clear();
						break;
					}
				}
			}
		}
		else if (event.mouseButton.button == sf::Mouse::Button::Right) {
			if (item_plant) {
				inventory->AddItem(item_plant->getItemId());
				item_plant = nullptr;
				game_state->setActionInfo(ActionInfo::none);
			}
		}
	}
	if (event.type == sf::Event::MouseButtonReleased) {
		if (item_move) {
			if (inv_butt && inv_butt->getOpen()) {
				inventory->AddItem(item_move->getItemId());
				DeleteEntity(item_move->getId());
			}
			if (check_compost_boxes) {
				for (auto e : entity_hovered) {
					if (e->getType() == COMPOST_BOX) {
						auto cb = FindCompostBox(e->getId());
						cb->AddBioJunk();
						DeleteEntity(item_move->getId());
						break;
					}
				}
			}
			item_move = nullptr;
		}
		else if (compost_box_move) {
			compost_box_move = nullptr;
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

bool World::getCanUseTool(int tool, Item::ItemType& icon)
{
	if (tool != -1 && !item_place && !item_move && !item_plant) {
		auto t = Item::Manager::getTool(tool);
		string name = t->name;

		if (name == "Bol") {
			auto b = Item::Manager::getBowl(tool);
			GroundType gtype = ground.getTileClickedType(mouse_pos_in_world);

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
		else if (name == "Faux") {
			GroundTile* gtile = ground.getTileClicked(mouse_pos_in_world);
			auto gtype = gtile->getType();

			if (gtype == GroundType::GRASS) {
				return true;
			}
		}
		else if (name == "Canne à pêche") {
			auto gtile = ground.getTileClicked(mouse_pos_in_world);
			if (gtile) {
				auto gtype = gtile->getType();

				if (gtype == GroundType::RIVER) {
					if (!fishing_shape.getWouldSnap(player->getPos() + player->getSize()/2.f, mouse_pos_in_world))
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
	else if (tool == -1 && !item_place && !item_move) {
		if (entity_hovered.size() != 0) {
			for (auto e : entity_hovered) {
				auto ent_type = e->getType();
				if (ent_type == APPLE_TREE) {
					icon = Item::ItemType::wood;
					return true;
				}
			}
		}

	}

	return false;
}

bool World::getCanCollect(Item::ItemType& item_type)
{
	if (item_place == nullptr && item_move == nullptr) {
		auto m = mouse_pos_in_world;
		for (auto t : trees) {
			auto tp = t->getPos();
			auto ts = t->getSize();

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
		for (auto cb : compost_boxes) {
			if (cb->getNbOfBags() > 0) {
				auto cp = cb->getPos();
				auto cs = cb->getSize();
				if (m.x > cp.x && m.x < cp.x + cs.x && m.y > cp.y && m.y < cp.y + cs.y) {
					item_type = Item::ItemType::compost_bag;
					return true;
				}
			}
		}
	}
	return false;
}

GroundTile * World::getGroundTileHovered(vec2 mpos)
{
	return ground.getTileClicked(mpos);
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
	for (auto cb : compost_boxes) {
		if (cb->getNbOfBags() > 0) {
			auto cp = cb->getPos();
			auto cs = cb->getSize();
			if (m.x > cp.x && m.x < cp.x + cs.x && m.y > cp.y && m.y < cp.y + cs.y) {
				vec2 pos;
				pos.x = rng::rand_float(cp.x, cp.x+cs.x);
				pos.y = rng::rand_float(cp.y, cp.y + 2);
				float end_y = pos.y + cs.y/2.f;
				particle_manager.CreateItemParticle(Item::ItemType::compost_bag, pos, end_y);
				cb->TakeOneBag();
				break;
			}
		}
	}
}

void World::DropItemFromInventory(int id)
{
	vec2 pos;
	vec2 pp = player->getPos();
	vec2 ps = player->getSize();
	pos.x = rng::rand_float(pp.x + ps.x/6.f, pp.x+ps.x - ps.x/6.f);
	pos.y = rng::rand_float(pp.y + ps.y/6.f, pp.y+ps.y/3.f);
	float end_y = pos.y + ps.y/2.f;
	vec2 move_vec;
	move_vec.x = rng::rand_float(-0.4f, 0.4f);
	move_vec.y = rng::rand_float(-0.3f, -0.1f);
	particle_manager.CreateItemParticle(id, pos, end_y, move_vec);
	particle_manager.SortItemParticles();
}

void HitTree(TreeObj* tree, Particle::Manager* particle_manager, World* world)
{
	auto tp = tree->getPos();
	auto ts = tree->getSize();
	// Create leaf particles
	int leaf_amount = rng::rand_int(7, 14);
	for (int i = 0; i != leaf_amount; ++ i) {
		vec2 pos;
		pos.x = rng::rand_float(tp.x + ts.x/6.f, tp.x+ts.x - ts.x/6.f);
		pos.y = rng::rand_float(tp.y + ts.y/6.f, tp.y+ts.y/3.f*2.f);
		float end_y = pos.y + ts.y/2.f;
		particle_manager->CreateSpriteParticle(Particle::SpriteParticleType::Leaf, pos, end_y);
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
		particle_manager->CreateSpriteParticle(wood_part_type, pos, end_y);
	}

	if (tree->getChopped()) {
		auto items_dropped = tree->getDroppedItems();
		for (auto i : items_dropped) {
			for (int n = 0; n != i.second; ++n) {
				vec2 pos;
				pos.x = rng::rand_float(tp.x + ts.x/3.f, tp.x+ts.x - ts.x/3.f);
				pos.y = rng::rand_float(tp.y + ts.y/2.f, tp.y+ts.y/4.f*3.5f);
				float end_y = pos.y + ts.y/2.f;
				particle_manager->CreateItemParticle(i.first, pos, end_y);
			}
		}
		world->DeleteTree(tree->getId());
	}

	particle_manager->SortSpriteParticles();
	particle_manager->SortItemParticles();
}

void World::UseEquippedToolAt()
{
	auto m = mouse_pos_in_world;
	int t = game_state->getEquippedTool();
	if (t != -1) {
		auto tool = Item::Manager::getTool(t);

		if (tool->name == "Bol") {
			auto b = Item::Manager::getBowl(t);
			GroundType gtype = ground.getTileClickedType(mouse_pos_in_world);

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

		if (tool->name == "Faux") {
			auto gtile = ground.getTileClicked(mouse_pos_in_world);

			if (gtile->getType() == GroundType::GRASS) {
				gtile->setType(GroundType::DIRT);
				ground.ReloadTileMap();
			}
		}

		// TREE CUTTING
		if (tool->name == "Hache") {
			for (auto tree : trees) {
				auto tp = tree->getPos();
				auto ts = tree->getSize();
				if (m.x > tp.x && m.x < tp.x + ts.x && m.y > tp.y && m.y < tp.y + ts.y) {
					tree->Hit();
					HitTree(tree, &particle_manager, this);
				}
			}
		}

		if (tool->name == "Canne à pêche") {
			fishing = true;
			fishing_shape.setStart(player->getPos());
			ground.StartRipple(mouse_pos_in_world, sf::seconds(2.f), 2.f);
			fishing_shape.setEnd(m);
			game_state->setActionInfo(ActionInfo::fishing);
		}
	}
	else { // no tool equipped
		for (auto tree : trees) {
			auto tp = tree->getPos();
			auto ts = tree->getSize();
			if (m.x > tp.x && m.x < tp.x + ts.x && m.y > tp.y && m.y < tp.y + ts.y) {
				tree->Hit();
				HitTree(tree, &particle_manager, this);
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

void World::DeleteCarrotPlant(int id)
{
	for (int i = 0; i != entities.size(); ++i) {
		if (entities[i]->getId() == id) {
			delete entities[i];
			entities.erase(entities.begin() + i);
			break;
		}
	}
	for (int i = 0; i != carrot_plants.size(); ++i) {
		if (carrot_plants[i]->getId() == id) {
			delete carrot_plants[i];
			carrot_plants.erase(carrot_plants.begin() + i);
			break;
		}
	}
}

void World::DeleteCompostBox(int id)
{
	for (int i = 0; i != entities.size(); ++i) {
		if (entities[i]->getId() == id) {
			delete entities[i];
			entities.erase(entities.begin() + i);
			break;
		}
	}
	for (int i = 0; i != compost_boxes.size(); ++i) {
		if (compost_boxes[i]->getId() == id) {
			delete compost_boxes[i];
			compost_boxes.erase(compost_boxes.begin() + i);
			break;
		}
	}
}

CompostBox * World::FindCompostBox(int id)
{
	for (auto cb : compost_boxes) {
		if (cb->getId() == id) return cb;
	}
	return nullptr;
}

void World::StartPlaceItem(ItemObject* item) {
	item_place = item;
	game_state->setActionInfo(ActionInfo::place);
}

void World::StartPlantItem(ItemObject * item)
{
	item_plant = item;
	game_state->setActionInfo(ActionInfo::plant);
}

void World::StartPlaceCompostBox(CompostBox * cb)
{
	AddCompostBox(cb);
	compost_box_place = cb;
	game_state->setActionInfo(ActionInfo::place_compost_box);
}

void World::PlantSeed()
{
	auto i = Item::Manager::getAny(item_plant->getItemId());
	auto it = Item::getItemTypeByName(i->name);
	auto p = item_plant->getPos() + item_plant->getSize() / 2.f;

	if (it == Item::ItemType::apple_seed) {
		auto tree = make_tree_obj(APPLE_TREE, 1, p);
		AddTreeEnt(tree);
	}
	else if (it == Item::ItemType::banana_seed) {
		auto tree = make_tree_obj(BANANA_TREE, 1, p);
		AddTreeEnt(tree);
	}
	else if (it == Item::ItemType::carrot_seed) {
		auto cp = make_carrot_plant(p - vec2(visual_tile_size/2.f, visual_tile_size/2.f));
		AddCarrotPlant(cp);
	}
}

void World::SortEntitiesImpl() // seems to be super fast for not a lot of elements (~10µs)
{
	sort(entities.begin(), entities.end(), [](auto e1, auto e2) {
		return e1->getPos().y + e1->getSize().y < e2->getPos().y + e2->getSize().y;
	});
}

