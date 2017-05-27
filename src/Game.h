#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>

#include "StateMachine.h"
#include "Globals.h"

#include <map>

namespace ConsoleNamespace { class Console; };
class GameState;
class MenuState;

static sf::Time QUICK_EXIT_TIME = sf::seconds(0.3f);


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

	sf::RenderTarget& getTarget() { return render_texture; }
	sf::RenderWindow& getWindow() { return window; }
	vec2 getMousePos();
	ConsoleNamespace::Console& getConsole() { return *console; }
	Controls& getControls() { return game_settings.controls; }
	State getActiveState() { return state_machine.getActiveState(); }
	GameState& getGameState() { return *game_state; }
	bool getSmallMode() { return small_mode; }

private:
	GameSettings game_settings;
	sf::RenderWindow window;

	bool small_mode = false;
	
	sf::RenderTexture render_texture;
	sf::Sprite render_sprite;

	ConsoleNamespace::Console* console;

	StateMachine state_machine;
	GameState* game_state;
	MenuState* menu_state;

	bool show_fps_counter = true;

	sf::Clock quit_timer;
};