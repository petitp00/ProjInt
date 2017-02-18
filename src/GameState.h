#pragma once

#include "Game.h"

class GameState
{
public:
	GameState();
	~GameState();

	void Update();
	void Render(sf::RenderTarget& target);

	bool getActive() { return active; }
	void setActive(bool active) { this->active = active; }

private:
	bool active = false;
};
