#include "World.h"

#include "GameState.h"
#include "ResourceManager.h"
#include "rng.h"

#include <fstream>
#include <iostream>
using namespace std;

Controls* controls;


GroundTile::GroundTile(GroundType type, sf::Vector2f pos) :
	type(type), pos(pos)
{ }


void Ground::LoadTileMap(const int * tiles, unsigned width, unsigned height)
{
	static const int tile_size = 64;
	static const int visual_size = 64 * 1.5;

	tileset = &ResourceManager::getTexture("Placeholders/Ground.png");

	vertices.setPrimitiveType(sf::Quads);
	vertices.resize(width*height*4);

	for (unsigned i = 0; i != width; ++i) {
		for (unsigned j = 0; j != height; ++j) {
			int tile_nb = tiles[i+j*width];

			int u, v = 0;
			u = rng::rand_int(0, 2);
			if (tile_nb == GRASS) { v = 0; }
			if (tile_nb == SAND) { v = 1; }

			sf::Vertex* quad = &vertices[(i+j*width) * 4];

			quad[0].position = sf::Vector2f(i*visual_size, j*visual_size);
			quad[1].position = sf::Vector2f((i+1)*visual_size, j*visual_size);
			quad[2].position = sf::Vector2f((i+1)*visual_size, (j+1)*visual_size);
			quad[3].position = sf::Vector2f(i*visual_size, (j+1)*visual_size);

			quad[0].texCoords = sf::Vector2f(u*tile_size, v*tile_size);
			quad[1].texCoords = sf::Vector2f((u+1)*tile_size, v*tile_size);
			quad[2].texCoords = sf::Vector2f((u+1)*tile_size, (v+1)*tile_size);
			quad[3].texCoords = sf::Vector2f(u*tile_size, (v+1)*tile_size);
		}
	}
}

void Ground::Clear()
{
	tiles.clear();
}

void Ground::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
	states.texture = tileset;
	target.draw(vertices, states);
}

World::World(GameState & game_state, Controls* controls) :
	game_state(game_state),
	game_view(sf::FloatRect({0,0}, {float(WINDOW_WIDTH), float(WINDOW_HEIGHT)}))

{
	//LoadWorld("test");
	//CreateAndSaveWorld("test");
	//player->setControls(controls);
	::controls = controls;
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
	int t[w*h] ={};

	for (int i = 0; i != w*h; ++i) {
		if (i % 4 == 0) t[i] = 1;
		else t[i] = 0;
	}

	ground.LoadTileMap(t, w, h);

	player = new Player();
	player->setControls(controls);

	entities.push_back(new GameObject({700, 400}, "box.png", {0,0}, SOLID));
	entities.push_back(player);

	Save();
}

void World::LoadWorld(std::string const & filename)
{
	Clear();
	name = filename;
	std::ifstream s("Resources/Data/Saves/" + filename);

	std::string w, str;

	while (s >> w) {
		if (w == "e") {
			Entity::Type t;
			s >> str;
			t = Entity::Type(atoi(str.c_str()));

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
			case Entity::ENTITY:
				cerr << "ERROR in save file: \"Data/Saves/" << filename << "\": entity saved as type Entity, which is a pure virtual class." << endl;
				break;
			case Entity::PLAYER:
				pl = new Player(p, sz, f, vec);
				player = pl;
				entities.push_back(player);
				break;
			case Entity::GAME_OBJECT:
				go = new GameObject(p, sz, f, vec);
				entities.push_back(go);
				break;
			default:
				break;
			}
		}
	}
	player->setControls(controls);

}

void World::Save()
{
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

	for (auto e : entities) {
		s << "e " << e->getType() << " "
			<< e->getPos().x << " " << e->getPos().y << " "
			<< e->getSize().x << " " << e->getSize().y << " "
			<< e->getFlags() << " ";
		for (auto & str : e->getSavedData()) { s << '"' << str << "\" "; }
		s << endl;
	}

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
	target.setView(game_view);

	target.draw(ground);

	for (auto e : entities) {
		e->Render(target);
	}

	target.setView(target.getDefaultView());
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
