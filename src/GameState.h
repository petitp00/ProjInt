#pragma once

#include "Game.h"

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <SFML/Graphics/Text.hpp>

class GameState
{
public:
	GameState();
	~GameState();

	void Update(float dt);
	void Render(sf::RenderTarget& target);

	void KeyPressedEvent(sf::Keyboard::Key key);
	void MousePressedEvent(sf::Mouse::Button button, int mouse_x, int mouse_y);
	
	bool getActive() { return active; }
	void setActive(bool active) { this->active = active; }

private:
	bool active = false;


	sf::Text wew;
};
