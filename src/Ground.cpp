#include "Ground.h"
#include "ResourceManager.h"
#include "rng.h"
#include "Globals.h"

#include <deque>
#include <map>
#include <iostream>
#include <sstream>

using namespace std;

vector<pair<unsigned long, vec2i>> ott1;
vector<pair<GroundType, vector<vec2i>>> types;

int getVariationMax(GroundType type) {
	for (auto t : types) {
		if (t.first == type) {
			return t.second.size()-1;
		}
	}
	return 0;
}

Tileset::Tileset(std::string filename)
{
	auto tiletexture = ResourceManager::getTexture(filename);
	sf::Image original_texture(tiletexture.copyToImage());

	ott1.push_back({UP,					{5, 6}});
	ott1.push_back({DOWN,				{4, 6}});
	ott1.push_back({LEFT,				{4, 7}});
	ott1.push_back({RIGHT,				{5, 7}});
	ott1.push_back({CORNER_TOP_LEFT,	{7, 7}});
	ott1.push_back({CORNER_TOP_RIGHT,	{6, 7}});
	ott1.push_back({CORNER_DOWN_LEFT,	{7, 6}});
	ott1.push_back({CORNER_DOWN_RIGHT,	{6, 6}});

	types.push_back({NONE,				{{0,0}}});
	types.push_back({GRASS,				{{1,0}, {2,0}, {2,1}, {3,1}, {4,1}}});
	types.push_back({SAND,				{{3,0}, {4,0}}});
	types.push_back({RIVER,				{{5,0}}});
	types.push_back({DRY_DIRT,			{{7,0}, {0,1}, {1,1} }});
	types.push_back({DIRT,				{{6,0}}});

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
	uint i = 0;
	int version = 0;
	for (auto& t : types) {
		if (t.first == main_type) break;
		++i;
	}
	if (i < types.size()) {
		version = rng::rand_int(0, types[i].second.size()-1);
	}

	vec->push_back({version, main_type, type_overlap_val[main_type]});
	for (auto o : overlaps) {
		vec->push_back({o.dir, o.type, type_overlap_val[o.type]});
	}
}

GroundTile::GroundTile(GroundType type, vec2 pos) : type(type), pos(pos) { }

std::string GroundTile::getName()
{
	switch (type)
	{
	case NONE: return "None";
	case GRASS: return "Gazon";
	case SAND: return "Sable";
	case RIVER: return "Rivière";
	case DRY_DIRT: return "Terre sèche";
	case DIRT: return "Terre";
	default: return "Missing case in GroundTile::getName()";
	}
}

Ground::Ground() : tileset("Ground.png")
{
	shader.loadFromFile("Resources/Shaders/TileShader.vert", "Resources/Shaders/TileShader.frag");

	for (int i = 0; i != RIPPLES_AMOUNT; ++i) {
		ripples[i] = Ripple();
	}
}

void Ground::LoadTileMap(std::vector<vector<int>> tiles, unsigned width, unsigned height)
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
			int tile_nb = tiles[i+j*width][0];

			this->tiles.emplace_back(GroundType(tile_nb), vec2(float(i), float(j)));

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

			vector<sf::Vector3i>* tnbvec = new vector<sf::Vector3i>; //x: texturex, y: texturey, z: h (overlap val)
			tileset.getUV(tnbvec, GroundType(tile_nb), overlaps);

			bool main_tile_checked = false;
			for (auto& t: *tnbvec) {
				QuadH q;
				sf::Color c = sf::Color(255, 255, 255, 255);
				if (!main_tile_checked && GroundType(tile_nb) == GroundType::RIVER) c.r = 0; // signals our shader to draw water on top
				main_tile_checked = true;

				q.verts.push_back(sf::Vertex(vec2(fi*vts,		fj*vts),		c, vec2(t.x*ts, t.y*ts)));
				q.verts.push_back(sf::Vertex(vec2((fi+1)*vts,	fj*vts),		c, vec2((t.x+1)*ts, t.y*ts)));
				q.verts.push_back(sf::Vertex(vec2((fi+1)*vts,	(fj+1)*vts),	c, vec2((t.x+1)*ts, (t.y+1)*ts)));
				q.verts.push_back(sf::Vertex(vec2(fi*vts,		(fj+1)*vts),	c, vec2(t.x*ts, (t.y+1)*ts)));
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

void Ground::ReloadShader()
{
	shader.loadFromFile("Resources/Shaders/TileShader.vert", "Resources/Shaders/TileShader.frag");
	cout << "RELOAD SHADER" << endl;
}

void Ground::Clear()
{
	tiles.clear();
}

void Ground::Fill(vec2 mpos, GroundType type)
{
	vec2 tpos{
		float(int(mpos.x / visual_tile_size)),
		float(int(mpos.y / visual_tile_size))
	};
	auto target_tile = getTile(tpos);
	if (!target_tile) return;
	GroundType target_type = target_tile->getType(); // contiguous tiles of this type will be replaced
	if (!type || type == target_type) return;

	std::deque<vec2> stack;
	std::vector<int> to_remove;
	stack.push_back(tpos);

	while (stack.size()) {
		int _i = 0; // keeps track of the current index in stack
		//for (auto n : stack) {
		for (int stack_index = 0; stack_index != stack.size(); ++stack_index) {
			auto n = stack[stack_index];
			to_remove.push_back(_i); // we remove every tile we check from the stack (after the looping through the stack)

			// find the boundaries of the line
			auto w = n.x;
			auto e = n.x;
			while (w > 0) {
				w -= 1;
				auto gt = getTile(w, n.y);
				if (!gt || gt->getType() != target_type) {
					++w;
					break;
				}
			}
			while (e < width-1) {
				e += 1;
				auto gt = getTile(e, n.y);
				if (!gt || gt->getType() != target_type) {
					--e;
					break;
				}
			}

			bool check_north = (n.y > 0);
			bool check_south = (n.y < height-1);

			for (float i = w; i != e + 1; ++i) {
				getTile(i, n.y)->setType(type);
				if (check_north && getTile(i, n.y - 1)->getType() == target_type) {
					stack.push_back({float(i), n.y-1});
				}
				if (check_south && getTile(i, n.y + 1)->getType() == target_type) {
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

void Ground::setTileClicked(vec2 mpos, GroundType type)
{
	vec2 tpos;
	tpos.x = float(int(mpos.x / visual_tile_size));
	tpos.y = float(int(mpos.y / visual_tile_size));
	
	if (tpos.x >= 0 && tpos.x < width && tpos.y >= 0 && tpos.y < height) {
		int max = getVariationMax(type);
		int var = rng::rand_int(0, max);

		getTile(tpos)->setType(type);
		getTile(tpos)->setVariation(var);
		ReloadTileMap();
	}
}

void Ground::StartRipple(vec2 pos, sf::Time duration, float strength)
{
	SoundManager::Play("waterSplouch.wav");
	auto find_lowest_ripple = [&]() {
		int lowest = -1;
		float lowest_val = 99999;
		for (int i = 0; i != RIPPLES_AMOUNT; ++i) {
			auto r = ripples[i];
			if (r.duration == sf::Time::Zero) {
				lowest = i;
				break;
			}
			auto val = r.duration.asSeconds() - r.start_time.asSeconds();
			if (val < lowest_val) {
				lowest_val = val;
				lowest = i;
			}
		}

		if (lowest != -1) {
			return lowest;
		}
		else return 0;
	};

	auto& r = ripples[find_lowest_ripple()];
	r.pos = pos;
	r.duration = duration;
	r.duration = sf::seconds(5);
	r.start_time = clock.getElapsedTime();
}

void Ground::UpdateRipples(float dt)
{
	auto t = sf::microseconds(int(dt*1000.f));

	for (int i = 0; i != RIPPLES_AMOUNT; ++i) {
		auto& r = ripples[i];
		//r.duration -= t;
		//if (r.duration < sf::Time::Zero) r.duration = sf::Time::Zero;
	}
}

std::string Ground::getSaveString()
{
	stringstream ss;

	for (auto t : tiles) {
		ss << t.getType() << " ";
		ss << t.getVariation() << " ";
		ss << t.getHumidity() << " ";
		ss << t.getFertility() << " , ";
	}

	return ss.str();
}

GroundType Ground::getTileClickedType(vec2 mpos)
{
	auto tile_clicked = getTileClicked(mpos);
	if (tile_clicked)
		return tile_clicked->getType();
	else
		return GroundType::NONE;
}

GroundTile* Ground::getTile(vec2 pos)
{
	if (pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= height) return nullptr;
	uint i = uint(pos.x + pos.y*width);
	if (i < tiles.size()) {
		return &tiles[i];
	}
	return nullptr;
}

GroundTile* Ground::getTile(float x, float y)
{
	if (x < 0 || x >= width || y < 0 || y >= height) return nullptr;
	uint i = uint(x + y*width);
	if (i < tiles.size()) {
		return &tiles[i];
	}
	return nullptr;
}

GroundTile* Ground::getTileClicked(vec2 mpos)
{
	vec2 tpos;
	tpos.x = float(int(mpos.x / visual_tile_size));
	tpos.y = float(int(mpos.y / visual_tile_size));
	return getTile(tpos);
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

	sf::Glsl::Vec4 rips[RIPPLES_AMOUNT];
	for (int i = 0; i != RIPPLES_AMOUNT; ++i) {
		auto r = ripples[i];
		rips[i].x = r.pos.x;
		rips[i].y = r.pos.y;
		rips[i].z = r.duration.asSeconds();
		rips[i].w = r.start_time.asSeconds();
		//cout << rips[i].z << " " << rips[i].w << endl;
	}

	shader.setUniform(unif, time);
	shader.setUniformArray("ripples", rips, RIPPLES_AMOUNT);
	states.shader = &shader;
	target.draw(vertices, states);
}
