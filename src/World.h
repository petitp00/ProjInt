#pragma once

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <vector>

#include "Entity.h"

/*
What we need to save:
	- every entity
	- player pos, health, etc.
	- inventory
*/

class GameState;

enum GroundType {
	GRASS,
	SAND
};
	
class GroundTile {
public:

	GroundTile(GroundType type, sf::Vector2f pos);

	GroundType getType() { return type; }
	sf::Vector2f getPos() { return pos; }

private:
	GroundType type;
	sf::Vector2f pos;
};

class Ground : public sf::Drawable
{
public:
	void LoadTileMap(std::vector<int> tiles, unsigned width, unsigned height);
	void Clear();

	std::vector<GroundTile>& getTiles() { return tiles; }
	int getWidth() { return width; }
	int getHeight() { return height; }
private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::VertexArray vertices;
	std::vector<GroundTile> tiles;
	sf::Texture* tileset;
	int width, height;
};

class World
{
public:
	World(GameState& game_state, Controls* controls);
	~World();

	void Clear();
	void CreateAndSaveWorld(std::string const& filename);
	void LoadWorld(std::string const& filename);
	void Save();

	void Update(float dt);
	void Render(sf::RenderTarget& target);

	bool HandleEvent(sf::Event const& event);

	sf::View& getGameView() { return game_view; }
	Entity* FindEntityClicked(sf::Vector2f mpos);

private:
	GameState& game_state;
	std::string name;

	sf::View game_view;

	Ground ground;
	Player* player = nullptr;
	std::vector<Entity*> entities;
};
