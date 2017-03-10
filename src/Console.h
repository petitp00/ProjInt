#pragma once

#include "Game.h"
#include "Tweener.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

#include <deque>
#include <vector>
#include <functional>


namespace ConsoleNamespace {

	static int CONSOLE_HEIGHT = WINDOW_HEIGHT / 3;
	static std::string CONSOLE_FONT = "consola.ttf";
	static uint CONSOLE_FONT_SIZE = 18;
	static int CONSOLE_ALPHA = int(0.85 * 255);

	static sf::Color TEXT_COLOR		= sf::Color(180, 180, 180, CONSOLE_ALPHA);
	static sf::Color TEXT_COLOR2	= sf::Color(254, 254, 255, CONSOLE_ALPHA);
	static sf::Color BG_COLOR		= sf::Color(41,  52,  71,  CONSOLE_ALPHA);
	static sf::Color BG_COLOR2		= sf::Color(26,  34,  45,  CONSOLE_ALPHA);
	static sf::Color RESULT_COLOR	= sf::Color(255, 205, 54);
	static sf::Color INFO_COLOR		= sf::Color(54,  197, 219);
	static sf::Color ERROR_COLOR	= sf::Color(237, 68,  59);

	enum LineMode {
		COMMAND,
		RESULT,
		INFO,
		ERROR,
	};

	static unsigned int MAX_AMOUNT_OF_LINES = 24;


	/* Commands to implement:
		+ OpenEditor	- to open the map editor. Also makes new commands available
		+ help
		+
	*/

	struct CommandActionImpl {
		CommandActionImpl(
			Console& console, Game& game, MenuState& menu_state, GameState& game_state) :
			console(console), game(game), menu_state(menu_state), game_state(game_state) {}
		
		Console& console;
		Game& game;
		MenuState& menu_state;
		GameState& game_state;
	};

	using caction_t = std::function<void(CommandActionImpl* impl, const std::vector<std::string>&)>;

	struct Command {
		Command(std::string name, caction_t action) : name(name), action(action) {}
		std::string name; //eg.: help, clear, set, etc.

		std::string desc = "No description available";
		std::string help_string = "No help available for: " + name;

		// arguments passed in a vector
		caction_t action;
	};

	class Console
	{
	public:
		Console(Game& game, MenuState& menu_state, GameState& game_state);
		~Console();

		void Init();
		void Update();
		void Render(sf::RenderTarget& target);
		void ParseAndExecute();
		bool HandleEvent(const sf::Event& event);

		// Getters
		bool getActive() { return active; }
		std::vector<Command*>& getCommands() { return commands; }
		Command* getCommand(const std::string& name);

		// Setters
		void setActive(bool active);

		// Functions for commands
		void ClearLines();
		void AddInfo(const std::string& name, const std::string& desc, const std::string& help);

	private:
		bool active = false;
		bool big_mode = false;
		bool waiting_on_input = false;

		CommandActionImpl command_action_impl;
		std::vector<Command*> commands;
		void InitCommands();

		void AddLine(std::string text, LineMode mode = COMMAND);
		void UpdateLinesOrigin();
		std::deque<sf::Text*> lines;

		// Graphical stuff
		float ypos = -float(CONSOLE_HEIGHT);
		Tweener ypos_tw;
		sf::RectangleShape main_shape;
		sf::RectangleShape input_shape;


		// Input stuff
		void UpdateInputTextObj();
		void UpdateInputCaret(bool update_pos = false);

		std::string input_string;
		sf::Text input_text_obj_greater_then;
		sf::Text input_text_obj;

		sf::RectangleShape caret_shape;
		int caret_pos = 0;
		bool draw_caret = true;
		sf::Clock caret_clock;
		sf::Time caret_blink_time = sf::seconds(0.4f);


	};

};
