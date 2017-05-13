#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include "Globals.h"

static const int tile_size = 32;
static const int visual_tile_size = int(64);
static const int RIPPLES_AMOUNT = 16;

enum GroundType { 
	NONE = 0,
	GRASS = 1,
	SAND = 2,
	RIVER = 3,
	DRY_DIRT = 4,
	DIRT = 5,

};

static const int ground_type_max = DIRT;
static std::string getGroundTypeString(GroundType type) {
	switch (type) {
	case NONE: return "NONE";
	case GRASS: return "GRASS";
	case SAND: return "SAND";
	case RIVER: return "RIVER";
	case DRY_DIRT: return "DRY_DIRT";
	case DIRT: return "DIRT";
	default: return "???";
	}
}

static std::map<GroundType, int> type_overlap_val = {
	{NONE, 0},
	{RIVER, 10},
	{DRY_DIRT, 20},
	{DIRT, 25},
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

struct Overlap {
	GroundType type;
	int dir;
};

class Tileset {
public:
	Tileset(std::string filename);
	void getUV(std::vector<sf::Vector3i>* vec, GroundType main_type, std::vector<Overlap>& overlaps);
	sf::Texture const& getTexture() const { return texture; }
private:
	sf::Texture texture;
};
	
class GroundTile {
public:
	GroundTile(GroundType type, vec2 pos);

	void setType(GroundType type) { this->type = type; }
	void setVariation(int var) { variation = var; }

	std::string getName();

	GroundType getType() { return type; }
	vec2 getPos() { return pos; }
	int getVariation() { return variation; }
	int getHumidity() { return humidity; }
	int getFertility() { return fertility; }

private:
	GroundType type;
	vec2 pos;
	int variation = 0;
	int humidity = 0; // [0, 100]
	int fertility = 0; // [0, 100]
};

struct Ripple {
	vec2 pos = { 0,0 };
	sf::Time duration = sf::seconds(0);
	float strength = 0; // from 0 to 1
};

class Ground : public sf::Drawable
{
public:
	Ground();
	void LoadTileMap(std::vector<std::vector<int>> tiles, unsigned width, unsigned height);
	void ReloadTileMap();

	void ReloadShader();

	void Clear();
	void Fill(vec2 mpos, GroundType type);
	void setTileClicked(vec2 mpos, GroundType type);

	void StartRipple(vec2 pos, sf::Time duration, float strength);
	void UpdateRipples(float dt);

	// Getters
	std::string getSaveString();
	GroundType getTileClickedType(vec2 mpos);
	GroundTile* getTile(vec2 pos);
	GroundTile* getTile(float x, float y);
	GroundTile* getTileClicked(vec2 mpos);
	std::vector<GroundTile>& getTiles()		{ return tiles; }
	int getWidth()							{ return width; }
	int getHeight()							{ return height; }
	static float getVisualTileSize();

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	mutable sf::Shader shader; // mutable: can be modified in const functions
	sf::Clock clock;
	Ripple ripples[RIPPLES_AMOUNT];

	sf::VertexArray vertices;
	std::vector<GroundTile> tiles;
	sf::Texture* tileset_texture;
	int width, height;

	Tileset tileset;

	// returned when getTile receives bad pos
	GroundTile not_found{NONE, {-1, -1}};
};
