#pragma once

#include "Game.h"

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <SFML/Graphics/Text.hpp>

#include "World.h"

class GameState
{
public:
	GameState(Controls* controls);
	~GameState();

	void Update(float dt);
	void Render(sf::RenderTarget& target);

	bool HandleEvent(sf::Event const& event);

	void StartNewGame(std::string const& name);

	bool getActive() { return active; }
	void setActive(bool active) { this->active = active; }

private:
	bool active = false;

	World world;

};
