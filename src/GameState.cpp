#include "GameState.h"

#include "Console.h"
#include "rng.h"
#include "ResourceManager.h"

#include <iostream>
using namespace std;

static sf::Vector2f mouse_pos_in_world = {-1, -1};

GameState::GameState(Game& game) :
	game(game),
	inventory(&game.getControls()),
	world(&game.getControls())
{
}

GameState::~GameState()
{
}

void GameState::Init(ButtonActionImpl * button_action_impl)
{
	inventory.Init(button_action_impl);
	inv_butt.Init(&inventory);
	world.Init(&inventory, &inv_butt);
}

void GameState::Update(float dt)
{
	if (!game.getConsole().getActive())
	world.Update(dt, mouse_pos_in_world);
	inventory.Update();
}

void GameState::Render(sf::RenderTarget & target)
{
	target.clear(sf::Color::White);

	world.Render(target);

	target.setView(target.getDefaultView());
	inv_butt.Render(target);

	inventory.Render(target);

}

bool GameState::HandleEvent(sf::Event const & event)
{
	mouse_pos_in_world = game.getWindow().mapPixelToCoords(sf::Mouse::getPosition(game.getWindow()), world.getGameView());

	if (game.getConsole().getActive()) {
		if (event.type == sf::Event::MouseButtonPressed) {
			if (event.mouseButton.button == sf::Mouse::Middle) {
				auto p = game.getWindow().mapPixelToCoords(sf::Mouse::getPosition(game.getWindow()), world.getGameView());
				auto e = world.FindEntityClicked(p);
				if (e) {
					game.getConsole().PrintInfo("Entity clicked");
					game.getConsole().PrintInfo("ID: " + to_string(e->getId()));
					game.getConsole().PrintInfo("Type: " + to_string(e->getType()) + "   (" + getEntityTypeString(e->getType()) + ")");
				}
			}
		}
	}
	else if (inventory.getActive()) {
		if (inventory.HandleEvents(event)) return true;
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
			inventory.setActive(false);
			return true;
		}
	}
	else {
		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == game.getControls().get("Inventaire")) {
				inventory.setActive(true);
				return true;
			}
		}
		inv_butt.HandleEvent(event);
	}
	return world.HandleEvent(event);
}

void GameState::StartNewGame(std::string const & name)
{
	world.CreateAndSaveWorld(name);
}

void GameState::LoadGame(std::string const & name)
{
	world.LoadWorld(name);
}
