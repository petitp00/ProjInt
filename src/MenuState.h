#pragma once

#include "Game.h"

#include "Text.h"

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

class MenuState
{
public:
	MenuState();
	~MenuState();

	void Update();
	void Render(sf::RenderTarget& target);

	void KeyPressedEvent(sf::Keyboard::Key key);
	void MousePressedEvent(sf::Mouse::Button button, int mouse_x, int mouse_y);

	bool getActive() { return active; }
	void setActive(bool active) { this->active = active; }

private:
	bool active = false;

	TextBox tb;
};
