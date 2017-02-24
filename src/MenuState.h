#pragma once

#include "Game.h"

#include "GUIObjects.h"

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

class MenuPage
{
public:
	MenuPage();
	virtual ~MenuPage();

	virtual void Update();
	virtual void Render(sf::RenderTarget& target);

	void MousePressedEvent(int mouse_x, int mouse_y);
	void MouseMovedEvent(int mouse_x, int mouse_y);

	void AddGUIObject(GUIObject* obj) { gui_objects.push_back(obj); }

protected:

	sf::RenderTexture tooltip_render_target; // on top
	sf::Sprite tooltip_render_target_sprite;

	std::vector<GUIObject*> gui_objects;
};


class MenuState
{
public:
	MenuState(Game& game);
	~MenuState();

	void Update();
	void Render(sf::RenderTarget& target);

	void KeyPressedEvent(sf::Keyboard::Key key);
	void MousePressedEvent(sf::Mouse::Button button, int mouse_x, int mouse_y);
	void MouseMovedEvent(int mouse_x, int mouse_y);

	bool getActive() { return active; }
	void setActive(bool active) { this->active = active; }

	void setActiveState(State active_state) { this->active_state = active_state; }

private:
	ButtonActionImpl button_action_impl;

	bool active = false;
	State active_state = State::MainMenu;

	void InitMainMenu();
	MenuPage main_menu;

	void InitInfoMenu();
	MenuPage info_menu;
};

