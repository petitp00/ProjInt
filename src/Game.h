#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

#include "StateMachine.h"
class GameState;
class MenuState;

static int WINDOW_WIDTH = 1280;
static int WINDOW_HEIGHT = 720;

struct GameSettings
{
	unsigned int AALevel = 8;
	bool VSync = true;
	bool Fullscreen = false;
};

class Game
{
public:
	Game();
	~Game();

	void Start();

	void ChangeActiveState(State new_state, State old_state);

private:
	GameSettings game_settings;
	sf::RenderWindow window;

	StateMachine state_machine;
	GameState* game_state;
	MenuState* menu_state;
};