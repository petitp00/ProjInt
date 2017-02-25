#pragma once

#include "Game.h"

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <SFML/Graphics/Text.hpp>

#include "World.h"

class GameState
{
public:
	GameState();
	~GameState();

	void Update(float dt);
	void Render(sf::RenderTarget& target);

	void HandleEvent(sf::Event const& event);
	
	bool getActive() { return active; }
	void setActive(bool active) { this->active = active; }

private:
	bool active = false;

	World world;

};
