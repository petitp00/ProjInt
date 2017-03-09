#pragma once

#include "Game.h"
#include "Tweener.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

#include <deque>

/*
	CONSOLE CLASS
		
		+ 

*/

namespace ConsoleNamespace {
	
	static std::string CONSOLE_FONT = "consola.ttf";
	static unsigned int CONSOLE_FONT_SIZE = 18;
	static float CONSOLE_ALPHA = 0.75f;
	static sf::Color TEXT_COLOR = sf::Color(180, 180, 180, 255*CONSOLE_ALPHA);
	static sf::Color TEXT_COLOR2 = sf::Color(254, 254, 255, 255*CONSOLE_ALPHA);
	static sf::Color BG_COLOR = sf::Color(41, 52, 71, 255*CONSOLE_ALPHA);
	static sf::Color BG_COLOR2 = sf::Color(26, 34, 45, 255*CONSOLE_ALPHA);

	static int CONSOLE_HEIGHT = WINDOW_HEIGHT / 3.f;

	enum LineMode {
		COMMAND,
		RESULT,
		INFO,
		ERROR,
	};

	static sf::Color RESULT_COLOR = sf::Color(255, 205, 54);
	static sf::Color INFO_COLOR = sf::Color(54, 197, 219);
	static sf::Color ERROR_COLOR = sf::Color(237, 68, 59);

	class Console
	{
	public:
		Console();
		~Console();

		void Init();

		void Update();
		void Render(sf::RenderTarget& target);

		void ParseAndExecute();

		bool HandleEvent(sf::Event const& event);

		// Getters
		bool getActive() { return active; }

		// Setters
		void setActive(bool active);

	private:
		void AddLine(std::string text, LineMode mode = COMMAND);
		std::deque<sf::Text*> lines;

		bool active = false;
		bool big_mode = false;

		float ypos = -CONSOLE_HEIGHT;
		Tweener ypos_tw;

		// Graphical stuff
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
		sf::Time caret_blink_time = sf::seconds(0.4);

	};

};
