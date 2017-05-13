#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <string>
#include <fstream>
#include <iostream>

#define M_PI 3.14159265358979323846

using uint = unsigned int;
using vec2 = sf::Vector2f;
using vec2f = sf::Vector2f;
using vec2i = sf::Vector2i;

#ifndef EDITOR_MODE
extern int WINDOW_WIDTH;// = 1280;
extern int WINDOW_HEIGHT;// = 720;
#endif
#ifdef EDITOR_MODE
extern int WINDOW_WIDTH;// = 1800;
extern int WINDOW_HEIGHT;// = 900;
#endif

extern bool window_active;

static const int WORLD_W = 1280*3;
static const int WORLD_H = 1280*3;

static const int INV_MAX = 7;

static std::string BASE_FONT_NAME = "Cousine-Regular.ttf";

enum FontSize
{
	TINY = 20,
	SMALL = 30,
	NORMAL = 40,
	LARGE = 50,
	BIG = 70,
};

// t is between 0 and 1
inline sf::Color LerpColor(sf::Color col1, sf::Color col2, float t)
{
	return sf::Color(
		int(col1.r + (col2.r - col1.r) * t),
		int(col1.g + (col2.g - col1.g) * t),
		int(col1.b + (col2.b - col1.b) * t),
		int(col1.a + (col2.a - col1.a) * t)
	);
}

inline vec2 normalize(vec2 const& vec) {
	float norm = sqrt(vec.x*vec.x + vec.y*vec.y);
	auto v = vec2(vec.x / norm, vec.y / norm);
	return v;
}

inline float dist(vec2 const& a, vec2 const& b) {
	float dist = sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
	return dist;
}

inline std::ostream& operator<<(std::ostream& os, vec2 const& vec) {
	os << "{" << vec.x << ", " << vec.y << "}";
	return os;
}

struct CoordsInfo {
	std::string entity_name;
	sf::IntRect texture_rect;
	sf::IntRect collision_rect;
	std::string texture_name;
};

static CoordsInfo getCoordsInfo(const std::string& name)
{
	CoordsInfo ci;

	std::string fn = "Resources/Data/SpriteCoords/" + name + ".txt";
	std::ifstream s;//(fn.c_str());
	s.open(fn);
	if (s.fail()) {
		std::cerr << "Cannot open coords info file \"" << fn  << "\"" << std::endl;
	}

	std::string w;

	s >> w;
	ci.entity_name = w;

	s >> w;
	ci.texture_rect.left = atoi(w.c_str());
	s >> w;
	ci.texture_rect.top = atoi(w.c_str());
	s >> w;
	ci.texture_rect.width = atoi(w.c_str());
	s >> w;
	ci.texture_rect.height = atoi(w.c_str());

	s >> w;
	ci.collision_rect.left = atoi(w.c_str());
	s >> w;
	ci.collision_rect.top = atoi(w.c_str());
	s >> w;
	ci.collision_rect.width = atoi(w.c_str());
	s >> w;
	ci.collision_rect.height = atoi(w.c_str());

	s >> w;
	ci.texture_name = w;

	return ci;
}
