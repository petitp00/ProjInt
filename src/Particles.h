#pragma once
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Time.hpp>

#include <vector>

#include "Globals.h"

namespace Particle
{
	struct Particle
	{
		virtual void Update(float dt) = 0;

		vec2 pos;
		float lifetime;
	};

	enum class SpriteParticleType {
		Leaf,
		AppleWood,
		BananaWood
	};

	struct SpriteParticle : public Particle
	{
		void Update(float dt) override;
		
		SpriteParticleType type;
		int var = 0;
		float end_y;
		vec2 move_vec;
		float angle;
		float scale;
	};
	void DrawSpriteParticle(SpriteParticle& part, sf::RenderTarget& target);

	class Manager
	{
	public:
		void Init();
		void Clear();
		void UpdateParticles(float dt);
		void RenderParticles(sf::RenderTarget& target);
		int RenderSpriteParticlesLowerThan(sf::RenderTarget& target, float y, int start_at = 0);

		void CreateSpriteParticle(SpriteParticleType type, vec2 pos, float end_y);
		void SortSpriteParticles(); // Sorting is based on end_y (no need to resort)

	private:
		std::vector<SpriteParticle> sprite_particles;
	};

}