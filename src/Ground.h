#pragma once

#include <string>
#include <SFML/Graphics.hpp>

static const int tile_size = 32;
static const int visual_tile_size = int(64*1.5);

enum GroundType { 
	NONE = 0,
	GRASS = 1,
	SAND = 2,
	STONES = 3,
	DRY_DIRT = 4
};

static const int ground_type_max = DRY_DIRT;
static std::string getGroundTypeString(GroundType type) {
	switch (type) {
	case NONE: return "NONE";
	case GRASS: return "GRASS";
	case SAND: return "SAND";
	case STONES: return "STONES";
	case DRY_DIRT: return "DRY_DIRT";
	default: return "???";
	}
}

static std::map<GroundType, int> type_overlap_val = {
	{NONE, 0},
	{STONES, 10},
	{DRY_DIRT, 20},
	{SAND, 30},
	{GRASS, 40}
};

// returns true if t2 overlaps on t1
static bool getTypeDominant(GroundType t1, GroundType t2, bool accept_equal = false) {
	if (accept_equal)
		return type_overlap_val[t2] >= type_overlap_val[t1];
	else
		return type_overlap_val[t2] > type_overlap_val[t1];
}
static bool getTypeDominant(int t1, int t2, bool accept_equal = false) {
	return getTypeDominant(GroundType(t1), GroundType(t2), accept_equal);
}
//
//#define NO_FLAG 0
//#define UP 1
//#define DOWN 2
//#define LEFT 4
//#define RIGHT 8
//#define CORNER_TOP_LEFT 16
//#define CORNER_TOP_RIGHT 32
//#define CORNER_DOWN_LEFT 64
//#define CORNER_DOWN_RIGHT 128

enum TYPE_DIRS {
	UP = 8,
	DOWN = 9,
	LEFT = 10,
	RIGHT = 11,
	CORNER_TOP_LEFT = 12,
	CORNER_TOP_RIGHT = 13, 
	CORNER_DOWN_LEFT = 14, 
	CORNER_DOWN_RIGHT = 15
};

struct Overlap
{
	GroundType type;
	int dir;
};

class Tileset {
public:
	Tileset(std::string filename);
	void getUV(std::vector<sf::Vector3i>* vec, GroundType main_type,
			   std::vector<Overlap>& overlaps);
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
