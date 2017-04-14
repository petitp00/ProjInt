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
	ott1.push_back({UP,					{5, 6}});
	ott1.push_back({DOWN,				{4, 6}});
	ott1.push_back({LEFT,				{4, 7}});
	ott1.push_back({RIGHT,				{5, 7}});
	ott1.push_back({CORNER_TOP_LEFT,	{7, 7}});
	ott1.push_back({CORNER_TOP_RIGHT,	{6, 7}});
	ott1.push_back({CORNER_DOWN_LEFT,	{7, 6}});
	ott1.push_back({CORNER_DOWN_RIGHT,	{6, 6}});

	vector<pair<GroundType, vector<sf::Vector2i>>> types;
	types.push_back({NONE,				{{0,0}}});
	types.push_back({GRASS,				{{1,0}}});
	types.push_back({SAND,				{{2,0}, {3,0}}});
	types.push_back({RIVER,			{{4,0}}});
	types.push_back({DRY_DIRT,			{{5,0}}});

	sf::Image image;
	image.create(uint(tile_size*16), uint(tile_size*16), sf::Color::Transparent);

	auto ts = tile_size;

	for (auto t : types) {
		int imgx = 0;
		int imgy = t.first;
		for (auto tp : t.second) { // copying full textures
			image.copy(original_texture, imgx*ts, imgy*ts, {tp.x*ts, tp.y*ts, ts, ts});
			++imgx;
		}
		
		if (t.first != NONE) {
			imgx = 15;
			for (int i = ott1.size()-1; i > -1; --i) {
				int px = 0, py = 0; // pixel pos in tile
				for (int p = 0; p != ts*ts; ++p) {
					auto mask_pixel = original_texture.getPixel(px + ott1[i].second.x*ts, py + ott1[i].second.y*ts);
					auto add = mask_pixel.g/3;
					if (mask_pixel.a != 0) {
						auto col = original_texture.getPixel(px + t.second[0].x*ts, py + t.second[0].y*ts);
						col.r = max(col.r-add, 0);
						col.g = max(col.g-add, 0);
						col.b = max(col.b-add, 0);
						image.setPixel(imgx*ts + px, imgy*ts + py, col);
					}

					if (px != tile_size-1) ++px;
					else { px = 0; ++py; }
				}
				--imgx;
			}
		}
		++imgy;
	}


	if (!image.saveToFile("Resources/test.png")) {
		cerr << "FAILED TO SAVE"<< endl;
	}

	texture.loadFromImage(image);
}

void Tileset::getUV(vector<sf::Vector3i>* vec, GroundType main_type, vector<Overlap>& overlaps)
{
	vec->push_back({0, main_type, type_overlap_val[main_type]});

	for (auto o : overlaps) {
		vec->push_back({o.dir, o.type, type_overlap_val[o.type]});
	}
}

GroundTile::GroundTile(GroundType type, sf::Vector2f pos) : type(type), pos(pos) { }

Ground::Ground() :
	tileset("Ground.png")
{
	shader.loadFromFile("Resources/Shaders/TileShader.vert", "Resources/Shaders/TileShader.frag");
}

void Ground::LoadTileMap(std::vector<int> tiles, unsigned width, unsigned height)
{
	this->tiles.clear();

	this->width = width;
	this->height = height;

	tileset_texture = &ResourceManager::getTexture("Ground.png");

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

struct QuadH {
	vector<sf::Vertex> verts;
	int h;
};

void Ground::ReloadTileMap()
{
	static std::vector<float> times;
	static sf::Clock clock;

	clock.restart();

	vector<QuadH> quadhs;
	quadhs.reserve(width*height);

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

			vector<Overlap> overlaps;

			unsigned long flags = 0;
			if (getTypeDominant(tile_nb, up)) overlaps.push_back({GroundType(up), UP});
			if (getTypeDominant(tile_nb, down)) overlaps.push_back({GroundType(down), DOWN});
			if (getTypeDominant(tile_nb, left)) overlaps.push_back({GroundType(left), LEFT});
			if (getTypeDominant(tile_nb, right)) overlaps.push_back({GroundType(right), RIGHT});

			if (getTypeDominant(tile_nb, up_left) && getTypeDominant(up, up_left, true) && getTypeDominant(left, up_left, true))
					overlaps.push_back({GroundType(up_left), CORNER_TOP_LEFT});
			if (getTypeDominant(tile_nb, up_right) && getTypeDominant(up, up_right, true) && getTypeDominant(right, up_right, true))
					overlaps.push_back({GroundType(up_right), CORNER_TOP_RIGHT});
			if (getTypeDominant(tile_nb, down_left) && getTypeDominant(down, down_left, true) && getTypeDominant(left, down_left, true))
					overlaps.push_back({GroundType(down_left), CORNER_DOWN_LEFT});
			if (getTypeDominant(tile_nb, down_right) && getTypeDominant(down, down_right, true) && getTypeDominant(right, down_right, true))
					overlaps.push_back({GroundType(down_right), CORNER_DOWN_RIGHT});

			vector<sf::Vector3i>* tnbvec = new vector<sf::Vector3i>;
			tileset.getUV(tnbvec, GroundType(tile_nb), overlaps);

			bool main_tile_checked = false;
			for (auto& t: *tnbvec) {
				QuadH q;
				sf::Color c = sf::Color(255, 255, 255, 255);
				if (!main_tile_checked && GroundType(tile_nb) == GroundType::RIVER) c.r = 0; // signals our shader to draw water on top
				main_tile_checked = true;

				q.verts.push_back(sf::Vertex(sf::Vector2f(fi*vts, fj*vts), c, sf::Vector2f(t.x*ts, t.y*ts)));
				q.verts.push_back(sf::Vertex(sf::Vector2f((fi+1)*vts, fj*vts), c, sf::Vector2f((t.x+1)*ts, t.y*ts)));
				q.verts.push_back(sf::Vertex(sf::Vector2f((fi+1)*vts, (fj+1)*vts), c, sf::Vector2f((t.x+1)*ts, (t.y+1)*ts)));
				q.verts.push_back(sf::Vertex(sf::Vector2f(fi*vts, (fj+1)*vts), c, sf::Vector2f(t.x*ts, (t.y+1)*ts)));
				q.h = t.z;
				quadhs.push_back(q);
			}
			delete tnbvec;
		}
	}

	vertices.clear();
	vertices.resize(quadhs.size()*4+1);

	sort(quadhs.begin(), quadhs.end(), [](auto a, auto b) {return a.h < b.h;});

	int i2 = 0;
	for (uint i = 0; i != quadhs.size(); ++i) {
		vertices[i2+0] = quadhs[i].verts[0];
		vertices[i2+1] = quadhs[i].verts[1];
		vertices[i2+2] = quadhs[i].verts[2];
		vertices[i2+3] = quadhs[i].verts[3];
		i2 += 4;
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

GroundTile& Ground::getTile(sf::Vector2f pos)
{
	uint i = uint(pos.x + pos.y*width);
	if (i < tiles.size()) {
		return tiles[i];
	}
	return not_found;
}

GroundTile& Ground::getTile(float x, float y)
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

	std::string unif = "time";
	float time = clock.getElapsedTime().asSeconds();

	shader.setUniform(unif, time);
	states.shader = &shader;
	target.draw(vertices, states);
}

