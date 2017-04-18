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

namespace EditorMode { class Editor; }

/*
What we need to save:
	- every entity
	- player pos, health, etc.
	- inventory
*/

class GameState;
class Inventory;

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

	sf::View& getGameView() { return game_view; }
	Entity* FindEntityClicked(vec2 mpos);
	Entity* getEntity(int id);
	ItemObject* FindItem(int id);

	void UseEquippedToolAt(vec2 mouse_pos_in_world);
	void AddEntity(Entity* e) { if (e) entities.push_back(e); }
	void DuplicateEntity(int id);
	void DeleteEntity(int id);
	void DeleteItemObj(int id);
	void StartPlaceItem(ItemObject* item);

private:
	Controls* controls = nullptr;
	Inventory* inventory = nullptr;
	GameState* game_state = nullptr;
	InventoryButton* inv_butt;
	std::string name;

	sf::View game_view;

	Ground ground;
	Player* player = nullptr;
	std::vector<Entity*> entities;
	std::vector<ItemObject*> items; // also in entities

	Entity* entity_hovered = nullptr;
	ItemObject* item_place = nullptr;
	ItemObject* item_move = nullptr;
};
