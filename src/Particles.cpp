#include "Particles.h"
#include "ResourceManager.h"
#include "rng.h"

#include <iostream>
using namespace std;

sf::Sprite leaf_particle_sprite;
float leaf_grav = 0.005f;

void Particle::LeafParticle::Update(float dt)
{
	if (pos.y < end_y) {
		move_vec.y += leaf_grav*dt;
		pos += move_vec * dt;
	}
}

void Particle::DrawLeafParticle(LeafParticle & part, sf::RenderTarget& target)
{
	leaf_particle_sprite.setPosition(part.pos);
	leaf_particle_sprite.setRotation(part.angle);
	leaf_particle_sprite.setScale(part.scale, part.scale);

	target.draw(leaf_particle_sprite);
}

void Particle::Manager::Init()
{
	Clear();

	auto lcinfo = getCoordsInfo("leafparticle1");
	leaf_particle_sprite.setTexture(ResourceManager::getTexture(lcinfo.texture_name));
	leaf_particle_sprite.setTextureRect(lcinfo.texture_rect);
}

void Particle::Manager::Clear()
{
	leaves.clear();
}

void Particle::Manager::UpdateParticles(float dt)
{
	for (auto i = leaves.begin(); i != leaves.end();) {
		i->Update(dt);
		i->lifetime -= dt;

		if (false && i->lifetime <= 0) {
			leaves.erase(i);
			continue;
		}
		++i;
	}
}

void Particle::Manager::RenderParticles(sf::RenderTarget & target)
{
	for (auto i : leaves) {
		DrawLeafParticle(i, target);
	}
}

int Particle::Manager::RenderLeafParticlesLowerThan(sf::RenderTarget& target, float y, int start_at)
{
	for (int i = start_at; i < leaves.size(); ++i) {
		if (leaves[i].end_y >= y) return i;
		DrawLeafParticle(leaves[i], target);
	}
}

void Particle::Manager::CreateLeafParticle(vec2 pos, float end_y)
{
	LeafParticle l;
	l.lifetime = 5000;
	l.pos = pos;
	l.end_y = end_y;
	l.move_vec = vec2(rng::rand_float(-0.2f, 0.2f), -0.1f);
	l.angle = rng::rand_float(0.f, 360.f);
	l.scale = rng::rand_float(0.5f, 0.7f);
	leaves.push_back(l);
}

void Particle::Manager::SortLeafParticles() 
{
	sort(leaves.begin(), leaves.end(), [](auto a, auto b) {
		return a.end_y < b.end_y;
	});
}
