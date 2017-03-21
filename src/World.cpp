#include "World.h"

#include "GameState.h"
#include "ResourceManager.h"
#include "rng.h"
#include "Editor/Editor.h"

#include <deque>
#include <fstream>
#include <iostream>
using namespace std;

GroundTile::GroundTile(GroundType type, sf::Vector2f pos) : type(type), pos(pos) { }

static const int tile_size = 64;
static const int visual_tile_size = int(64 * 1.5);

void Ground::LoadTileMap(std::vector<int> tiles, unsigned width, unsigned height)
{
	this->tiles.clear();

	this->width = width;
	this->height = height;

	tileset = &ResourceManager::getTexture("Placeholders/Ground.png");

	vertices.setPrimitiveType(sf::Quads);
	vertices.resize(width*height*4);

	for (unsigned j = 0; j != height; ++j) {
		for (unsigned i = 0; i != width; ++i) {
			int tile_nb = tiles[i+j*width];

			this->tiles.emplace_back(GroundType(tile_nb), sf::Vector2f(float(i), float(j)));

			int u, v = 0;
			u = rng::rand_int(0, 2);
			if (tile_nb == NONE) { u = 3; }
			if (tile_nb == GRASS) { v = 0; }
			if (tile_nb == SAND) { v = 1; }

			sf::Vertex* quad = &vertices[(i+j*width) * 4];

			quad[0].position = sf::Vector2f(float(i*visual_tile_size), float(j*visual_tile_size));
			quad[1].position = sf::Vector2f(float((i+1)*visual_tile_size), float(j*visual_tile_size));
			quad[2].position = sf::Vector2f(float((i+1)*visual_tile_size), float(j+1)*visual_tile_size);
			quad[3].position = sf::Vector2f(float(i*visual_tile_size), float(j+1)*visual_tile_size);

			quad[0].texCoords = sf::Vector2f(float(u*tile_size), float(v*tile_size));
			quad[1].texCoords = sf::Vector2f(float((u+1)*tile_size), float(v*tile_size));
			quad[2].texCoords = sf::Vector2f(float((u+1)*tile_size), float(v+1)*tile_size);
			quad[3].texCoords = sf::Vector2f(float(u*tile_size), float(v+1)*tile_size);
		}
	}
}

void Ground::ReloadTileMap()
{
	vertices.clear();
	vertices.setPrimitiveType(sf::Quads);
	vertices.resize(width*height*4);

	for (unsigned j = 0; j != height; ++j) {
		for (unsigned i = 0; i != width; ++i) {
			int tile_nb = tiles[i+j*width].getType();

			int u, v = 0;
			u = rng::rand_int(0, 2);
			if (tile_nb == NONE) { u = 3; }
			if (tile_nb == GRASS) { v = 0; }
			if (tile_nb == SAND) { v = 1; }

			sf::Vertex* quad = &vertices[(i+j*width) * 4];

			quad[0].position = sf::Vector2f(float(i*visual_tile_size), float(j*visual_tile_size));
			quad[1].position = sf::Vector2f(float((i+1)*visual_tile_size), float(j*visual_tile_size));
			quad[2].position = sf::Vector2f(float((i+1)*visual_tile_size), float(j+1)*visual_tile_size);
			quad[3].position = sf::Vector2f(float(i*visual_tile_size), float(j+1)*visual_tile_size);

			quad[0].texCoords = sf::Vector2f(float(u*tile_size), float(v*tile_size));
			quad[1].texCoords = sf::Vector2f(float((u+1)*tile_size), float(v*tile_size));
			quad[2].texCoords = sf::Vector2f(float((u+1)*tile_size), float(v+1)*tile_size);
			quad[3].texCoords = sf::Vector2f(float(u*tile_size), float(v+1)*tile_size);
		}
	}
}

void Ground::Clear()
{
	tiles.clear();
}

void Ground::Fill(sf::Vector2f mpos, GroundType type)
{
	sf::Vector2f tpos{
		float(int(mpos.x / visual_tile_size)),
		float(int(mpos.y / visual_tile_size))
	};
	GroundType target_type = getTile(tpos).getType();
	if (type == target_type) return;

	std::deque<sf::Vector2f> stack;
	std::vector<int> to_remove;
	stack.push_back(tpos);

	int i = 0;
	while (stack.size()) {
		auto s = stack;
		int _i = 0;
		for (auto n : s) {
			to_remove.push_back(_i);
			auto w = n.x, e = n.x;
			while (w >= 0) {
				w -= 1;
				if (getTile(w, n.y).getType() != target_type) break;
			}
			while (e < width) {
				e += 1;
				if (getTile(e, n.y).getType() != target_type) break;
			}

			bool check_north = (n.y > 0);
			bool check_south = (n.y < height);

			for (float i = w+1; i != e; ++i) {
				getTile(i, n.y).setType(type);
				if (check_north && getTile(i, n.y - 1).getType() == target_type) {
					stack.push_back({float(i), n.y-1});
				}
				if (check_south && getTile(i, n.y + 1).getType() == target_type) {
					stack.push_back({float(i), n.y+1});
				}
			}
			++_i;
		}

		int nb_of_removals = 0;
		for (auto t : to_remove) {
			stack.erase(stack.begin() + t - nb_of_removals);
			++nb_of_removals;
		}
		to_remove.clear();
	}

	ReloadTileMap();
}

void Ground::setTileClicked(sf::Vector2f mpos, GroundType type)
{
	sf::Vector2f tpos;
	tpos.x = float(int(mpos.x / visual_tile_size));
	tpos.y = float(int(mpos.y / visual_tile_size));

	if (tpos.x >= 0 && tpos.x < width && tpos.y >= 0 && tpos.y < height) {
		getTile(tpos).setType(type);
		ReloadTileMap();
	}
}

GroundType Ground::getTileClicked(sf::Vector2f mpos)
{
	sf::Vector2f tpos;
	tpos.x = float(int(mpos.x / visual_tile_size));
	tpos.y = float(int(mpos.y / visual_tile_size));

	return getTile(tpos).getType();

	return NONE;
}

GroundTile & Ground::getTile(sf::Vector2f pos)
{
	uint i = uint(pos.x + pos.y*width);
	if (i < tiles.size()) {
		return tiles[i];
	}
	return not_found;
}

GroundTile & Ground::getTile(float x, float y)
{
	uint i = uint(x + y*width);
	if (i < tiles.size()) {
		return tiles[i];
	}
	return not_found;
}

float Ground::getVisualTileSize()
{
	return float(visual_tile_size);
}

void Ground::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
	states.texture = tileset;
	target.draw(vertices, states);
}

World::World():
	game_view(sf::FloatRect({0,0}, {float(WINDOW_WIDTH), float(WINDOW_HEIGHT)}))
{
}

World::World(Controls* controls) :
	game_view(sf::FloatRect({0,0}, {float(WINDOW_WIDTH), float(WINDOW_HEIGHT)})),
	controls(controls)

{
}

World::~World()
{
	Clear();
}

void World::Clear()
{
	for (int i = 0; i != entities.size(); ++i) {
		delete entities[i];
	}
	entities.clear();
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

	vector<int> t(EditorMode::WORLD_W/visual_tile_size * EditorMode::WORLD_H/visual_tile_size, NONE);
	ground.LoadTileMap(t, EditorMode::WORLD_W/visual_tile_size, EditorMode::WORLD_H/visual_tile_size);

	Save();
}

void World::LoadWorld(std::string const & filename)
{
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
			default:
				break;
			}
		}
		else if (w == "[TILE_MAP]") {
			cout << "   done! (" << clock.getElapsedTime().asSeconds() << " s)" << endl << "loading tile_map...";
			vector<int> tiles;
			int width, height;
			s >> width;
			s >> height;
			while (s >> w) {
				tiles.push_back(stoi(w));
			}

			ground.LoadTileMap(tiles, width, height);
		}
	}
	cout << "   done! (" << clock.getElapsedTime().asSeconds() << " s)" << endl;
	player->setControls(controls);

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
	cout << " - done! (" << clock.getElapsedTime().asSeconds() << " s)" << endl;;

	clock.restart();

	cout << "saving tilemap...";
	s << "[TILE_MAP]" << endl;
	s << ground.getWidth() << endl << ground.getHeight() << endl;
	for (auto t : ground.getTiles()) {
		s << t.getType() << " ";
	}
	s << endl;
	cout << " - done! (" << clock.getElapsedTime().asSeconds() << " s)" << endl;;

	if (!file_existed) {
		std::ofstream stream("Resources/Data/Saves/all_saves", std::ios_base::app);
		stream << name << endl;
	}

	cout << "World saved to \"Resources/Data/Saves/" << name << "\"" << endl;
}

void World::Update(float dt)
{
	for (auto i = entities.begin(); i != entities.end();) {
		auto e = *i;
		if (!e->getDead()) {
			e->Update(dt);
		}
		else {
			i = entities.erase(i);
			continue;
		}
		++i;
	}

	player->DoCollisions(entities);
	player->DoMovement(dt);
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
		if (event.mouseButton.button == sf::Mouse::Button::Middle) {
			middle_pressed = true;
			drag_mouse_pos ={event.mouseButton.x, event.mouseButton.y};
		}
	}
	if (event.type == sf::Event::MouseButtonReleased) {
		if (event.mouseButton.button == sf::Mouse::Button::Middle) {
			middle_pressed = false;
		}
	}
	if (event.type == sf::Event::MouseMoved) {
		if (middle_pressed) {
			sf::Vector2i m ={event.mouseMove.x, event.mouseMove.y};
			game_view.move(-sf::Vector2f(m - drag_mouse_pos));
			drag_mouse_pos = m;
		}
	}

	if (event.type == sf::Event::MouseWheelScrolled) {
		auto d = event.mouseWheelScroll.delta;
		game_view.zoom(1 + -d * 0.1f);
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
