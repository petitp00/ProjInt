#include "GameState.h"

#include "Console.h"
#include "rng.h"
#include "ResourceManager.h"

#include <iostream>
using namespace std;

GameState::GameState(Game& game) :
	game(game),
	world(*this, &game.getControls())
{
}

GameState::~GameState()
{
}

void GameState::Update(float dt)
{
	if (!game.getConsole().getActive())
	world.Update(dt);
}

void GameState::Render(sf::RenderTarget & target)
{
	target.clear(sf::Color::White);

	world.Render(target);
}

bool GameState::HandleEvent(sf::Event const & event)
{
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
