#include "World.h"

#include "GameState.h"

#include <fstream>
#include <iostream>
using namespace std;

World::World(GameState & game_state, Controls* controls) :
	game_state(game_state),
	game_view(sf::FloatRect({ 0,0 }, { float(WINDOW_WIDTH), float(WINDOW_HEIGHT) }))

{
	LoadWorld("test");
	//CreateAndSaveWorld("test");
	player->setControls(controls);
}

World::~World()
{
	for (int i = 0; i != entities.size(); ++i) {
		delete entities[i];
	}
	entities.clear();
}

void World::CreateAndSaveWorld(std::string const & filename)
{
	player = new Player();

	entities.push_back(new GameObject({ 700, 400 }, "box.png", { 0,0 }, SOLID));
	entities.push_back(player);

	Save("test");
}

void World::LoadWorld(std::string const & filename)
{
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
}

void World::Save(std::string const& filename)
{
	std::ifstream f("Resources/Data/Saves/" + filename, std::ios::binary);
	std::ofstream backup("Resources/Data/Saves/" + filename + ".backup", std::ios::binary);

	backup << f.rdbuf();
	backup.close();
	f.close();

	std::ofstream s("Resources/Data/Saves/" + filename);

	for (auto e : entities) {
		s << "e " << e->getType() << " "
			<< e->getPos().x << " " << e->getPos().y << " "
			<< e->getSize().x << " " << e->getSize().y << " "
			<< e->getFlags() << " ";
		for (auto & str : e->getSavedData()) { s << '"' << str << "\" ";}
		s << endl;
	}

	cout << "World save to \"Resources/Data/Saves/" << filename << "\"" << endl;
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
			Save("test");
		}
	}
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Button::Middle) {
			middle_pressed = true;
			drag_mouse_pos = { event.mouseButton.x, event.mouseButton.y };
		}
	}
	if (event.type == sf::Event::MouseButtonReleased) {
		if (event.mouseButton.button == sf::Mouse::Button::Middle) {
			middle_pressed = false;
		}
	}
	if (event.type == sf::Event::MouseMoved) {
		if (middle_pressed) {
			sf::Vector2i m ={ event.mouseMove.x, event.mouseMove.y };
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

