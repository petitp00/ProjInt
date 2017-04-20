#pragma once

#include "Game.h"

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/Text.hpp>

#include "World.h"
#include "GameGUI.h"

class GameState
{
public:
	GameState(Game& game);
	~GameState();

	void Init(ButtonActionImpl* button_action_impl);
	void Update(float dt);
	void Render(sf::RenderTarget& target);

	bool HandleEvent(sf::Event const& event);

	void StartNewGame(std::string const& name);
	void LoadGame(std::string const& name);

	void EquipTool(int id);// { equipped_tool = id; tool_obj.setTool(id); }
	void UnequipTool(); // { equipped_tool = -1; tool_obj.setTool(-1); }

	Inventory* getInventory() { return &inventory; }
	World& getWorld() { return world; }
	EquippedToolObj* getEquippedToolObj() { return &tool_obj; }
	bool getActive() { return active; }
	int getEquippedTool() { return equipped_tool; }
	void setActive(bool active) { this->active = active; }

private:
	bool active = false;

	Game& game;

	World world;

	int equipped_tool = -1;
	bool can_use_tool = false;
	bool using_tool = false;
	sf::Clock using_tool_clock;
	sf::RectangleShape using_tool_bar1;
	sf::RectangleShape using_tool_bar2;
	sf::Sprite equipped_tool_sprite;

	// GUI stuff
	Inventory inventory;
	InventoryButton inv_butt;
	EquippedToolObj tool_obj;

};
