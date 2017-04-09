#pragma once

#include "Tweener.h"
#include "Items.h"
#include <SFML/Graphics.hpp>

class GUIObject;
class InvItemButton;
class TextBox;
class TextButton;
class InvActionButton;
class ButtonActionImpl;
struct Controls;

static sf::Color INV_WINDOW_COLOR(115, 106, 91);
static sf::Color INV_ACCENT_COLOR(144, 131, 114);
static sf::Color INV_ACCENT_COLOR2(135, 118, 96);
static sf::Color INV_ACCENT_COLOR3(169, 145, 98);
static sf::Color INV_TEXT_COLOR(47, 50, 55);

class Inventory
{
public:
	Inventory(Controls* controls);
	~Inventory();
	void Init(ButtonActionImpl* button_action_impl);

	void Update();
	void Render(sf::RenderTarget& target);
	bool HandleEvents(sf::Event const& event);
	void ResetItemButtons();
	void ResetItemDescription(bool item_selected = true);
	void AddItem(Item::any item);
	void RemoveItem(Item::any item);
	void EatItem(Item::any item);
	void PutDownItem(Item::any item);

	bool getActive() { return active; }
	std::vector<Item::any>& getItems() { return items; }

	void setActive(bool active);

private:
	Controls* controls;
	ButtonActionImpl* button_action_impl;
	bool active = false;

	std::vector<Item::any> items;

	sf::RenderTexture tooltip_render_target; // on top
	sf::Sprite tooltip_render_target_sprite;

	std::vector<GUIObject*> gui_objects;
	std::vector<InvItemButton*> inv_buttons;
	std::vector<InvActionButton*> actions_buttons;
	sf::RectangleShape window_shape;
	PosTweener window_tw;
	sf::RectangleShape item_desc_shape;
	TextBox* item_desc_obj;

	Item::any selected_item;
};

class InventoryButton
{
public:
	InventoryButton();
	void Init(Inventory* inventory);

	void Render(sf::RenderTarget& target);
	void HandleEvent(sf::Event const& event);

	bool getOpen() { return open; }

private:
	Inventory* inventory;

	sf::Sprite sprite;
	sf::Vector2f pos;
	sf::Vector2f size;

	void UpdateOpen();
	bool open = false;
};