#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

#include "StateMachine.h"
class GameState;
class MenuState;

static int WINDOW_WIDTH = 1280;
static int WINDOW_HEIGHT = 720;

static std::string BASE_FONT_NAME = "blue highway rg.ttf";

enum FontSize
{
	TINY = 20,
	SMALL = 30,
	NORMAL = 40,
	BIG = 70,
};

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