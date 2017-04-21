#include "Particles.h"
#include "ResourceManager.h"
#include "rng.h"

#include <iostream>
using namespace std;

sf::Sprite leaf_particle_sprite;
float leaf_grav = 0.005f;

map<Particle::SpriteParticleType, vector<sf::IntRect>> trects;

void Particle::SpriteParticle::Update(float dt)
{
	if (pos.y < end_y) {
		move_vec.y += leaf_grav*dt;
		pos += move_vec * dt;
	}
}

void Particle::DrawSpriteParticle(SpriteParticle & part, sf::RenderTarget& target)
{
	leaf_particle_sprite.setPosition(part.pos);
	leaf_particle_sprite.setRotation(part.angle);
	leaf_particle_sprite.setScale(part.scale, part.scale);
	leaf_particle_sprite.setTextureRect(trects[part.type][part.var-1]);

	target.draw(leaf_particle_sprite);
}

void Particle::Manager::Init()
{
	Clear();

	auto leaf_cinfo = getCoordsInfo("leafparticle1");
	leaf_particle_sprite.setTexture(ResourceManager::getTexture(leaf_cinfo.texture_name));

	trects[SpriteParticleType::Leaf] = {leaf_cinfo.texture_rect};

	auto awood1_cinfo = getCoordsInfo("awoodpart1");
	auto awood2_cinfo = getCoordsInfo("awoodpart2");
	auto awood3_cinfo = getCoordsInfo("awoodpart3");
	auto awood4_cinfo = getCoordsInfo("awoodpart4");
	trects[SpriteParticleType::AppleWood] = {
		awood1_cinfo.texture_rect,
		awood2_cinfo.texture_rect,
		awood3_cinfo.texture_rect,
		awood4_cinfo.texture_rect
	};

	auto bwood1_cinfo = getCoordsInfo("bwoodpart1");
	auto bwood2_cinfo = getCoordsInfo("bwoodpart2");
	auto bwood3_cinfo = getCoordsInfo("bwoodpart3");
	trects[SpriteParticleType::BananaWood] = {
		bwood1_cinfo.texture_rect,
		bwood2_cinfo.texture_rect,
		bwood3_cinfo.texture_rect
	};
}

void Particle::Manager::Clear()
{
	sprite_particles.clear();
}

void Particle::Manager::UpdateParticles(float dt)
{
	for (auto i = sprite_particles.begin(); i != sprite_particles.end();) {
		i->Update(dt);
		i->lifetime -= dt;

		if (false && i->lifetime <= 0) {
			sprite_particles.erase(i);
			continue;
		}
		++i;
	}
}

void Particle::Manager::RenderParticles(sf::RenderTarget & target)
{
	for (auto i : sprite_particles) {
		DrawSpriteParticle(i, target);
	}
}

int Particle::Manager::RenderSpriteParticlesLowerThan(sf::RenderTarget& target, float y, int start_at)
{
	for (int i = start_at; i < int(sprite_particles.size()); ++i) {
		if (sprite_particles[i].end_y >= y) return i;
		DrawSpriteParticle(sprite_particles[i], target);
	}

	return -1;
}

void Particle::Manager::CreateSpriteParticle(SpriteParticleType type, vec2 pos, float end_y)
{
	SpriteParticle p;
	p.type = type;
	
	if (type == SpriteParticleType::Leaf) {
		p.var = 1;
		p.scale = rng::rand_float(0.5f, 0.7f);
	}
	else if (type == SpriteParticleType::AppleWood) {
		p.var = rng::rand_int(1, 4);
		p.scale = rng::rand_float(1.f, 2.f);
	}
	else if (type == SpriteParticleType::BananaWood) {
		p.var = rng::rand_int(1, 3);
		p.scale = rng::rand_float(1.f, 2.f);
	}

	p.lifetime = 5000;
	p.pos = pos;
	p.end_y = end_y;
	p.move_vec = vec2(rng::rand_float(-0.2f, 0.2f), -0.1f);
	p.angle = rng::rand_float(0.f, 360.f);
	sprite_particles.push_back(p);
}

void Particle::Manager::SortSpriteParticles() 
{
	sort(sprite_particles.begin(), sprite_particles.end(), [](auto a, auto b) {
		return a.end_y < b.end_y;
	});
}
