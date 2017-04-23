#pragma once
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Time.hpp>

#include <vector>

#include "Globals.h"
#include "Items.h"

class World;

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
		BananaWood,
		WoodItem
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

	struct ItemParticle : public Particle
	{
		void Update(float dt) override;

		bool create_new;
		int existing_id = -1;
		Item::ItemType type;
		float end_y;
		vec2 move_vec;
		float scale;
	};
	void DrawItemParticle(ItemParticle& part, sf::RenderTarget& target);

	class Manager
	{
	public:
		void Init(World* world);
		void Clear();
		void UpdateParticles(float dt);

		int RenderSpriteParticlesLowerThan(sf::RenderTarget& target, float y, int start_at = 0);
		void CreateSpriteParticle(SpriteParticleType type, vec2 pos, float end_y);
		void SortSpriteParticles(); // Sorting is based on end_y (no need to resort)

		int RenderItemParticlesLowerThan(sf::RenderTarget& target, float y, int start_at = 0);
		void CreateItemParticle(Item::ItemType type, vec2 pos, float end_y, vec2 move_vec=vec2(0,0));
		void CreateItemParticle(int id, vec2 pos, float end_y, vec2 move_vec=vec2(0,0)); // for existing items

		void SortItemParticles();

	private:
		World* world = nullptr;

		std::vector<SpriteParticle> sprite_particles;
		std::vector<ItemParticle> item_particles;
	};

}