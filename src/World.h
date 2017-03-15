#pragma once

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <vector>
#include <functional>

#include "Entity.h"

namespace EditorMode { class Editor; }

/*
What we need to save:
	- every entity
	- player pos, health, etc.
	- inventory
*/

class GameState;

enum GroundType { 
	NONE = 0,
	GRASS = 1,
	SAND = 2
};

static const int ground_type_max = SAND;
static std::string getGroundTypeString(GroundType type) {
	switch (type) {
	case NONE: return "NONE";
	case GRASS: return "GRASS";
	case SAND: return "SAND";
	default: return "???";
	}
}
	
class GroundTile {
public:

	GroundTile(GroundType type, sf::Vector2f pos);

	void setType(GroundType type) { this->type = type; }

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
	void ReloadTileMap();
	void Clear();

	void setTileClicked(sf::Vector2f mpos, GroundType type);
	GroundType getTileClicked(sf::Vector2f mpos);
	std::vector<GroundTile>& getTiles() { return tiles; }
	int getWidth() { return width; }
	int getHeight() { return height; }
	
	static float getVisualTileSize();

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::VertexArray vertices;
	std::vector<GroundTile> tiles;
	sf::Texture* tileset;
	int width, height;
};

class World
{
	friend class EditorMode::Editor;
public:
	World();
	World(Controls* controls);
	~World();

	void Clear();
	void CreateAndSaveWorld(const std::string& filename);
	void CreateNewBlank(const std::string& filename);
	void LoadWorld(const std::string& filename);
	void Save(const std::string& filename ="");

	void Update(float dt);
	void Render(sf::RenderTarget& target);

	bool HandleEvent(sf::Event const& event);

	sf::View& getGameView() { return game_view; }
	Entity* FindEntityClicked(sf::Vector2f mpos);

	void AddEntity(Entity* e) { entities.push_back(e); }
	void DeleteEntity(int id);

private:
	Controls* controls;
	std::string name;

	sf::View game_view;

	Ground ground;
	Player* player = nullptr;
	std::vector<Entity*> entities;
};
