#include "Ground.h"
#include "ResourceManager.h"
#include "rng.h"
#include "Game.h" // for uint

#include <deque>
#include <map>
#include <iostream>

using namespace std;

Tileset::Tileset(std::string filename)
{
	auto tiletexture = ResourceManager::getTexture(filename);
	sf::Image original_texture(tiletexture.copyToImage());

	vector<pair<unsigned long, sf::Vector2i>> ott1;
	ott1.push_back({UP,				{1, 3}});
	ott1.push_back({DOWN,				{1, 5}});
	ott1.push_back({LEFT,				{0, 4}});
	ott1.push_back({RIGHT,				{2, 4}});
	ott1.push_back({CORNER_TOP_LEFT,	{4, 4}});
	ott1.push_back({CORNER_TOP_RIGHT,	{3, 4}});
	ott1.push_back({CORNER_DOWN_LEFT,	{4, 3}});
	ott1.push_back({CORNER_DOWN_RIGHT,	{3, 3}});

	sf::Image image;
	image.create(uint(tile_size*16), uint(tile_size*16), sf::Color::Transparent);

	int imgx = 0;
	int imgy = 0;

	image.copy(original_texture, imgx*tile_size, imgy*tile_size, {3*tile_size, 0, tile_size, tile_size}, true);
	++imgx;
	image.copy(original_texture, imgx*tile_size, imgy*tile_size, {0, 0, tile_size, tile_size}, true);
	++imgx;
	image.copy(original_texture, imgx*tile_size, imgy*tile_size, {0, tile_size, tile_size, tile_size}, true);
	++imgx;

	for (auto t : ott1) {
		int ix = 0;
		int iy = 0;
		for (int i = 0; i != tile_size * tile_size; ++i) {
			auto mask_pixel = original_texture.getPixel(ix + t.second.x*tile_size, iy + t.second.y*tile_size);
			if (mask_pixel.a != 0) {
				auto sand_pixel = original_texture.getPixel(ix, iy + tile_size);
				image.setPixel(imgx*tile_size + ix, imgy*tile_size + iy, sand_pixel);
			}
			
			if (ix != tile_size-1) ++ix;
			else { ix = 0; ++iy; }
		}

		if (imgx != tile_size-1) ++imgx;
		else { imgx = 0; ++imgy; }
	}

	if (!image.saveToFile("Resources/test.png")) {
		cerr << "FAILED TO SAVE"<< endl;
	}

	texture.loadFromImage(image);
}

void Tileset::getUV(vector<sf::Vector2i>* vec, GroundType main_type, GroundType second_type, unsigned long flags)
{
	if (main_type == NONE) {
		vec->push_back({0, 0});
	}
	else if (main_type == SAND) {
		vec->push_back({2,0});
	}
	else if (main_type == GRASS) {
		vec->push_back({1,0});
		if (second_type == SAND) {
			if ((flags & UP) != 0)					vec->push_back({3, 0});
			if ((flags & DOWN) != 0)				vec->push_back({4, 0});
			if ((flags & LEFT) != 0)				vec->push_back({5, 0});
			if ((flags & RIGHT) != 0)				vec->push_back({6, 0});
			if ((flags & CORNER_TOP_LEFT) != 0)		vec->push_back({7,0});
			if ((flags & CORNER_TOP_RIGHT) != 0)	vec->push_back({8,0});
			if ((flags & CORNER_DOWN_LEFT) != 0)	vec->push_back({9,0});
			if ((flags & CORNER_DOWN_RIGHT) != 0)	vec->push_back({10,0});
		}
	}
}

GroundTile::GroundTile(GroundType type, sf::Vector2f pos) : type(type), pos(pos) { }

Ground::Ground() :
	tileset("Placeholders/Ground.png")
{
}

void Ground::LoadTileMap(std::vector<int> tiles, unsigned width, unsigned height)
{
	this->tiles.clear();

	this->width = width;
	this->height = height;

	tileset_texture = &ResourceManager::getTexture("Placeholders/Ground.png");

	vertices.setPrimitiveType(sf::Quads);
	vertices.resize(width*height*4);

	vector<sf::Vertex> verts;

	for (unsigned j = 0; j != height; ++j) {
		for (unsigned i = 0; i != width; ++i) {
			int tile_nb = tiles[i+j*width];

			this->tiles.emplace_back(GroundType(tile_nb), sf::Vector2f(float(i), float(j)));

		}
	}

	ReloadTileMap();
}

void Ground::ReloadTileMap()
{
	static std::vector<float> times;
	static sf::Clock clock;

	clock.restart();

	vector<sf::Vertex> verts;
	verts.reserve(width*height);

	float vts = visual_tile_size;
	float ts = tile_size;

	for (unsigned j = 0; j != height; ++j) {
		for (unsigned i = 0; i != width; ++i) {
			float fi = float(i);
			float fj = float(j);
			int tile_nb = tiles[i+j*width].getType();

			int up = 0, down = 0, left = 0, right = 0;
			int up_left = 0, up_right = 0;
			int down_left = 0, down_right = 0;

			if (j != 0) {
				up = tiles[i+(j-1)*width].getType();
				if (i != 0) { up_left = tiles[i-1+(j-1)*width].getType(); }
				if (i != width - 1) { up_right = tiles[i+1+(j-1)*width].getType(); }
			}
			if (j != height - 1) {
				down = tiles[i+(j+1)*width].getType();
				if (i != 0) { down_left = tiles[i-1+(j+1)*width].getType(); }
				if (i != width - 1) { down_right = tiles[i+1+(j+1)*width].getType(); }
			}
			if (i != 0) { left = tiles[i-1+j*width].getType(); }
			if (i != width - 1) { right = tiles[i+1+j*width].getType(); }

			unsigned long flags = 0;
			if (up			== SAND) flags |= UP;
			if (down		== SAND) flags |= DOWN;
			if (left		== SAND) flags |= LEFT;
			if (right		== SAND) flags |= RIGHT;
			if (up_left		== SAND) flags |= CORNER_TOP_LEFT;
			if (up_right	== SAND) flags |= CORNER_TOP_RIGHT;
			if (down_left	== SAND) flags |= CORNER_DOWN_LEFT;
			if (down_right	== SAND) flags |= CORNER_DOWN_RIGHT;

			vector<sf::Vector2i>* tnbvec = new vector<sf::Vector2i>;
			tileset.getUV(tnbvec, GroundType(tile_nb), SAND, flags);

			for (auto& t: *tnbvec) {
				verts.emplace_back(sf::Vector2f(fi*vts, fj*vts), sf::Vector2f(t.x*ts, t.y*ts) );
				verts.emplace_back(sf::Vector2f((fi+1)*vts, fj*vts), sf::Vector2f((t.x+1)*ts, t.y*ts) );
				verts.emplace_back(sf::Vector2f((fi+1)*vts, (fj+1)*vts), sf::Vector2f((t.x+1)*ts, (t.y+1)*ts) );
				verts.emplace_back(sf::Vector2f(fi*vts, (fj+1)*vts), sf::Vector2f(t.x*ts, (t.y+1)*ts) );
			}
			delete tnbvec;
		}
	}

	vertices.clear();
	vertices.resize(verts.size());

	for (uint i = 0; i < verts.size(); ++i) {
		vertices[i] = verts[i];
	}
}

void Ground::Clear()
{
	tiles.clear();
}

void Ground::Fill(sf::Vector2f mpos, GroundType type)
{
	sf::Vector2f tpos{
		float(int(mpos.x / visual_tile_size)),
		float(int(mpos.y / visual_tile_size))
	};
	GroundType target_type = getTile(tpos).getType();
	if (type == target_type) return;

	std::deque<sf::Vector2f> stack;
	std::vector<int> to_remove;
	stack.push_back(tpos);

	int i = 0;
	while (stack.size()) {
		auto s = stack;
		int _i = 0;
		for (auto n : s) {
			to_remove.push_back(_i);
			auto w = n.x, e = n.x;
			while (w >= 0) {
				w -= 1;
				if (getTile(w, n.y).getType() != target_type) break;
			}
			while (e < width) {
				e += 1;
				if (getTile(e, n.y).getType() != target_type) break;
			}

			bool check_north = (n.y > 0);
			bool check_south = (n.y < height);

			for (float i = w+1; i != e; ++i) {
				getTile(i, n.y).setType(type);
				if (check_north && getTile(i, n.y - 1).getType() == target_type) {
					stack.push_back({float(i), n.y-1});
				}
				if (check_south && getTile(i, n.y + 1).getType() == target_type) {
					stack.push_back({float(i), n.y+1});
				}
			}
			++_i;
		}

		int nb_of_removals = 0;
		for (auto t : to_remove) {
			stack.erase(stack.begin() + t - nb_of_removals);
			++nb_of_removals;
		}
		to_remove.clear();
	}

	ReloadTileMap();
}

void Ground::setTileClicked(sf::Vector2f mpos, GroundType type)
{
	sf::Vector2f tpos;
	tpos.x = float(int(mpos.x / visual_tile_size));
	tpos.y = float(int(mpos.y / visual_tile_size));

	if (tpos.x >= 0 && tpos.x < width && tpos.y >= 0 && tpos.y < height) {
		getTile(tpos).setType(type);
		ReloadTileMap();
	}
}

GroundType Ground::getTileClicked(sf::Vector2f mpos)
{
	sf::Vector2f tpos;
	tpos.x = float(int(mpos.x / visual_tile_size));
	tpos.y = float(int(mpos.y / visual_tile_size));

	return getTile(tpos).getType();

	return NONE;
}

GroundTile & Ground::getTile(sf::Vector2f pos)
{
	uint i = uint(pos.x + pos.y*width);
	if (i < tiles.size()) {
		return tiles[i];
	}
	return not_found;
}

GroundTile & Ground::getTile(float x, float y)
{
	uint i = uint(x + y*width);
	if (i < tiles.size()) {
		return tiles[i];
	}
	return not_found;
}

float Ground::getVisualTileSize()
{
	return float(visual_tile_size);
}

void Ground::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
	states.texture = &tileset.getTexture();
	target.draw(vertices, states);
}

