#pragma once

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>

#include <vector>

#include "Entity.h"

class GameState;

class World
{
public:
	World(GameState& game_state);
	~World();

	void LoadWorld();

	void Update(float dt);
	void Render(sf::RenderTarget& target);

	bool HandleEvent(sf::Event const& event);

private:
	GameState& game_state;

	sf::View game_view;

	Player* player;
	std::vector<Entity*> entities;
};
