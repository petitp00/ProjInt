#pragma once

#include <SFML/Graphics/RenderWindow.hpp>

#include "StateMachine.h"
class GameState;
class MenuState;

static int WINDOW_WIDTH = 1280;
static int WINDOW_HEIGHT = 720;

//static std::string BASE_FONT_NAME = "blue highway rg.ttf";
static std::string BASE_FONT_NAME = "Cousine-Regular.ttf";

enum FontSize
{
	TINY = 20,
	SMALL = 30,
	NORMAL = 40,
	BIG = 70,
};

// t is between 0 and 1
inline sf::Color LerpColor(sf::Color col1, sf::Color col2, float t)
{
	return sf::Color(
		int(col1.r + (col2.r - col1.r) * t),
		int(col1.g + (col2.g - col1.g) * t),
		int(col1.b + (col2.b - col1.b) * t),
		int(col1.a + (col2.a - col1.a) * t)
	);
}

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
	void Quit();

	void ReturnToLastState() {
		auto s = state_machine.PopState();
		ChangeActiveState(state_machine.getActiveState(), s);
	}
	void ChangeActiveState(State new_state, State old_state);

	void ToggleFpsCounter();

private:
	GameSettings game_settings;
	sf::RenderWindow window;

	StateMachine state_machine;
	GameState* game_state;
	MenuState* menu_state;

	bool show_fps_counter = true;
};