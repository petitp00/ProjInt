#pragma once

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include <vector>
#include <functional>

#include "Entity.h"
#include "Ground.h"
#include "GameGUI.h"
#include "Particles.h"

namespace EditorMode { class Editor; }

/*
What we need to save:
	- every entity
	- player pos, health, etc.
	- inventory
*/

class GameState;
class Inventory;

class FishingPoleShape : public sf::Drawable
{
public:
	void setStart(vec2 start) {
		this->start = start;
		UpdateVertices();
	}
	void setEnd(vec2 end) {
		this->end = end;
		UpdateVertices();
	}

	vec2 getStart() { return start; }
	vec2 getEnd() { return end; }

	bool getShouldSnap() {
		float dist = sqrt(pow(start.x - end.x, 2) + pow(start.y - end.y, 2));
		return dist > max_dist;
	}

	bool getWouldSnap(vec2 start, vec2 end) {
		float dist = sqrt(pow(start.x - end.x, 2) + pow(start.y - end.y, 2));
		return dist > max_dist;
	}
	
private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		target.draw(vertices, states);
	}
	
	/*
		http://gizma.com/easing/
		t: current time,
		b: start value,
		c: change in value (delta),
		d: duration.
	*/

	void UpdateVertices() {
		using ltype = float(float, float, float, float);
		ltype* sin_in = [](float t, float b, float c, float d) {
			return -c * float(cos(t / d * (M_PI / 2))) + c + b;
		};
		ltype* sin_out = [](float t, float b, float c, float d) {
			return c * float(sin(t / d * (M_PI / 2))) + b;
		};
		ltype* quint_in = [](float t, float b, float c, float d) {
			t /= d;
			return c*t*t*t*t*t + b;
		};

		auto fx = end.y < start.y ? sin_out : sin_in;
		auto fy = end.y < start.y ? sin_in : sin_out;

		float dist = sqrt(pow(start.x - end.x, 2) + pow(start.y - end.y, 2));
		float dist_tweened = quint_in(dist / max_dist, 0, 0.7f, 1);

		sf::Color color1 = LerpColor(sf::Color(206, 190, 154), sf::Color(255, 0, 0), dist_tweened);
		sf::Color color2 = LerpColor(sf::Color(206+35, 190+35, 154+35), sf::Color(255, 0, 0), dist_tweened);

		float p_count = 30 + float(int(dist));

		vertices.clear();
		vertices.setPrimitiveType(sf::PrimitiveType::TrianglesStrip);

		for (float i = 0; i != p_count; ++i) {
			sf::Vertex v1, v2, v3, v4;

			float t = (i / p_count);
			sf::Color col = LerpColor(color1, color2, t);
			v1.color = col;
			v2.color = col;

			vec2 p1, p2;
			p1.x = (*fx)((i/p_count), start.x, end.x - start.x, 1.0f);
			p1.y = (*fy)((i/p_count), start.y, end.y - start.y, 1.0f);
			p2.x = (*fx)(((i+1)/p_count), start.x, end.x - start.x, 1.0f);
			p2.y = (*fy)(((i+1)/p_count), start.y, end.y - start.y, 1.0f);

			vec2 change( -(p2.y - p1.y), p2.x - p1.x );

			v1.position = p1 + change;
			v2.position = p1 - change;

			vertices.append(v1);
			vertices.append(v2);
		}
	}

private:
	bool active = false;

	vec2 start;
	vec2 end;

	float max_dist = 6*visual_tile_size;

	sf::VertexArray vertices;
};

class World
{
	friend class EditorMode::Editor;
public:
	World();
	World(Controls* controls);
	~World();

	void Init(Inventory* inventory, InventoryButton* inv_butt, GameState* game_state);
	void Clear();
	void CreateAndSaveWorld(const std::string& filename);
	void CreateNewBlank(const std::string& filename);
	void LoadWorld(const std::string& filename);
	void Save(const std::string& filename ="");

	void Update(float dt, vec2 mouse_pos_in_world);
	void UpdateView();
	void Render(sf::RenderTarget& target);
	bool HandleEvent(sf::Event const& event);

	sf::View getGameView();
	Entity* FindEntityClicked(vec2 mpos);
	Entity* getEntity(int id);
	ItemObject* FindItem(int id);
	TreeObj* FindTree(int id);
	CarrotPlant* FindCarrotPlant(int id);
	CompostBox* FindCompostBox(int id);
	bool getCanUseTool(int tool, Item::ItemType& icon);
	bool getCanCollect(Item::ItemType& item_type);
	GroundTile* getGroundTileHovered(vec2 mpose);

	void SortEntities() { entities_need_sorting = true; }
	void UseEquippedToolAt();
	void Collect();
	void DropItemFromInventory(int id);

	void AddEntity(Entity* e) { if (e) entities.push_back(e); }
	void AddItemEnt(ItemObject* i) { if (i) entities.push_back(i); items.push_back(i); }
	void AddTreeEnt(TreeObj* t) { if (t) entities.push_back(t); trees.push_back(t); }
	void AddCarrotPlant(CarrotPlant* cp) { entities.push_back(cp); carrot_plants.push_back(cp); }
	void AddCompostBox(CompostBox* cb) { entities.push_back(cb); compost_boxes.push_back(cb); }

	void DuplicateEntity(int id);

	void DeleteEntity(int id);
	void DeleteTree(int id);
	void DeleteItemObj(int id);
	void DeleteCarrotPlant(int id);
	void DeleteCompostBox(int id);

	void StartPlaceItem(ItemObject* item);
	void StartPlantItem(ItemObject* item);
	void StartPlaceCompostBox(CompostBox* cb);
	void PlantSeed();
	
private:
	void SortEntitiesImpl(); // called at beginning of Update if entities_need_sorting
	bool entities_need_sorting = false;

	Controls* controls = nullptr;
	Inventory* inventory = nullptr;
	GameState* game_state = nullptr;
	InventoryButton* inv_butt;
	std::string name;

	sf::View game_view;
	sf::RenderTexture render_texture;
	sf::Sprite render_sprite;

	sf::RectangleShape sleep_overlay;
	bool sleeping = false;
	void StartSleep();
	sf::Clock sleep_clock;
	sf::Time sleep_time;

	Ground ground;
	Particle::Manager particle_manager;
	Player* player = nullptr;
	std::vector<Entity*> entities;
	std::vector<ItemObject*> items; // also in entities
	std::vector<TreeObj*> trees; // also in entities
	std::vector<CarrotPlant*> carrot_plants; // also in entities
	std::vector<CompostBox*> compost_boxes; // also in entities

	std::vector<Entity*> entity_hovered;
	ItemObject* item_place = nullptr;
	ItemObject* item_move = nullptr;
	ItemObject* item_plant = nullptr;
	bool can_plant = false;

	CompostBox* compost_box_place = nullptr;
	CompostBox* compost_box_move = nullptr;
	
	// set to true when a compost box is hovered, check if we need to close it
	bool check_compost_boxes = false;

	FishingPoleShape fishing_shape;
	bool fishing = false;
	sf::Clock fishing_clock;

};
