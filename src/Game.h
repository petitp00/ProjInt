#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "StateMachine.h"

#include <map>

using uint = unsigned int;

namespace ConsoleNamespace { class Console; };
class GameState;
class MenuState;

#ifndef EDITOR_MODE
extern int WINDOW_WIDTH;// = 1280;
extern int WINDOW_HEIGHT;// = 720;
#endif
#ifdef EDITOR_MODE
extern int WINDOW_WIDTH;// = 1800;
extern int WINDOW_HEIGHT;// = 900;
#endif

//extern int WINDOW_W(int new_val = -1) {
//	static int winw = WINDOW_WIDTH;
//	if (new_val != -1) winw = new_val;
//	return winw;
//}
//extern int WINDOW_H(int new_val = -1) {
//	static int winh = WINDOW_HEIGHT;
//	if (new_val != -1) winh = new_val;
//	return winh;
//}
//
static sf::Time QUICK_EXIT_TIME = sf::seconds(0.3f);

static std::string BASE_FONT_NAME = "Cousine-Regular.ttf";

enum FontSize
{
	TINY = 20,
	SMALL = 30,
	NORMAL = 40,
	LARGE = 50,
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

inline sf::Vector2f normalize(sf::Vector2f const& vec) {
	float norm = sqrt(vec.x*vec.x + vec.y*vec.y);
	auto v = sf::Vector2f(vec.x / norm, vec.y / norm);
	return v;
}

inline float dist(sf::Vector2f const& a, sf::Vector2f const& b) {
	float dist = sqrt((a.x-b.x)*(a.x-b.x)+(a.y-b.y)*(a.y-b.y));
	return dist;
}

inline std::ostream& operator<<(std::ostream& os, sf::Vector2f const& vec) {
	os << "{" << vec.x << ", " << vec.y << "}";
	return os;
}

std::string getKeyString(sf::Keyboard::Key key);
char getKeyChar(sf::Event::KeyEvent e);

struct Controls
{
	void LoadDefault();
	void LoadUserControls();
	void SaveUserControls();

	sf::Keyboard::Key get(std::string s) {
		for (auto k : keys) {
			if (k.first == s) {
				return k.second;
			}
		}
		return sf::Keyboard::Unknown;
	}

	std::vector<std::pair<std::string, sf::Keyboard::Key>> keys;
};

struct GameSettings
{
	unsigned int AALevel = 8;
	bool VSync = true;
	bool Fullscreen = false;
	Controls controls;
};

////////////////////////////////////////////////////////////////////////////////////////////////

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

	sf::RenderWindow& getWindow() { return window; }
	ConsoleNamespace::Console& getConsole() { return *console; }
	Controls& getControls() { return game_settings.controls; }
	State getActiveState() { return state_machine.getActiveState(); }
	GameState& getGameState() { return *game_state; }

private:
	GameSettings game_settings;
	sf::RenderWindow window;

	ConsoleNamespace::Console* console;

	StateMachine state_machine;
	GameState* game_state;
	MenuState* menu_state;

	bool show_fps_counter = true;

	sf::Clock quit_timer;
};