#pragma once

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include <vector>

#include "Entity.h"

/*
What we need to save:
	- every entity
	- player pos, health, etc.
	- inventory
*/

class GameState;

class World
{
public:
	World(GameState& game_state, Controls* controls);
	~World();

	void CreateAndSaveWorld(std::string const& filename);

	void LoadWorld(std::string const& filename);
	void Save(std::string const& filename);

	void Update(float dt);
	void Render(sf::RenderTarget& target);

	bool HandleEvent(sf::Event const& event);

private:
	GameState& game_state;

	sf::View game_view;

	Player* player;
	std::vector<Entity*> entities;
};
