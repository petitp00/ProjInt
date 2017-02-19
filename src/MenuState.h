#pragma once

#include "Game.h"

#include "GUIObjects.h"

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

class MenuPage
{
public:
	MenuPage(Game& game);
	~MenuPage();

	void Update();
	void Render(sf::RenderTarget& target);

	void MousePressedEvent(int mouse_x, int mouse_y);
	void MouseMovedEvent(int mouse_x, int mouse_y);

private:
	sf::Text title;
	sf::Text tiny;
	sf::Text small;
	sf::Text normal;

	std::vector<GUIObject*> gui_objects;

	ButtonActionImpl button_action_impl;
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

private:
	bool active = false;

	MenuPage main_menu;
};

