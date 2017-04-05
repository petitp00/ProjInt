#pragma once

#include "Tweener.h"
#include "Items.h"
#include <SFML/Graphics.hpp>

class GUIObject;
class InvItemButton;
class TextBox;
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
	void Init();

	void Update();
	void Render(sf::RenderTarget& target);
	bool HandleEvents(sf::Event const& event);
	void ResetItemDescription();

	bool getActive() { return active; }

	void setActive(bool active);

private:
	Controls* controls;

	bool active = false;

	sf::RenderTexture tooltip_render_target; // on top
	sf::Sprite tooltip_render_target_sprite;

	std::vector<GUIObject*> gui_objects;
	std::vector<InvItemButton*> inv_buttons;

	sf::RectangleShape window_shape;
	PosTweener window_tw;

	sf::RectangleShape item_desc_shape;
	TextBox* item_desc_obj;

	Item::any selected_item;
};