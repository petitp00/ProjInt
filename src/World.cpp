#include "World.h"

#include "GameState.h"

#include <iostream>
using namespace std;

World::World(GameState & game_state) :
	game_state(game_state),
	game_view(sf::FloatRect({ 0,0 }, { float(WINDOW_WIDTH), float(WINDOW_HEIGHT) }))

{
	LoadWorld();
}

World::~World()
{
	for (int i = 0; i != entities.size(); ++i) {
		delete entities[i];
	}
	entities.clear();
}

void World::LoadWorld()
{
	player = new Player();

	entities.push_back(new GameObject({ 700, 400 }, "box.png", { 0,0 }, SOLID));
	//entities.push_back(new GameObject({ 300, 300 }, "box2.png", { 150,150 }, SOLID));
	entities.push_back(player);
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

void World::HandleEvent(sf::Event const & event)
{
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
}

