#pragma once

#include <string>
#include <SFML/Graphics.hpp>

static const int tile_size = 64;
static const int visual_tile_size = int(64 * 1.5);

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

#define NO_FLAG 0
#define UP 1
#define DOWN 2
#define LEFT 4
#define RIGHT 8
#define CORNER_TOP_LEFT 16
#define CORNER_TOP_RIGHT 32
#define CORNER_DOWN_LEFT 64
#define CORNER_DOWN_RIGHT 128

class Tileset {
public:
	Tileset(std::string filename);
	void getUV(std::vector<sf::Vector2i>* vec, GroundType main_type, GroundType second_type, unsigned long flags);
	sf::Texture const& getTexture() const { return texture; }

private:
	sf::Texture texture;

};
	
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
	Ground();
	void LoadTileMap(std::vector<int> tiles, unsigned width, unsigned height);
	void ReloadTileMap();
	void Clear();

	void Fill(sf::Vector2f mpos, GroundType type);
	void setTileClicked(sf::Vector2f mpos, GroundType type);
	GroundType getTileClicked(sf::Vector2f mpos);
	GroundTile& getTile(sf::Vector2f pos);
	GroundTile& getTile(float x, float y);
	std::vector<GroundTile>& getTiles() { return tiles; }
	int getWidth() { return width; }
	int getHeight() { return height; }
	
	static float getVisualTileSize();

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::VertexArray vertices;
	std::vector<GroundTile> tiles;
	sf::Texture* tileset_texture;
	int width, height;

	Tileset tileset;

	// returned when getTile receives bad pos
	GroundTile not_found{NONE, {-1, -1}};
};
