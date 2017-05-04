#include "Game.h"

#include <SFML/Window/Event.hpp>

#include "Console.h"
#include "Items.h"
#include "GameState.h"
#include "MenuState.h"
#include "ResourceManager.h"

#include <fstream>
#include <iostream>
using namespace std;


#ifndef EDITOR_MODE
int WINDOW_WIDTH = 1280;
int WINDOW_HEIGHT = 720;
#endif
#ifdef EDITOR_MODE
int WINDOW_WIDTH = 1800;
int WINDOW_HEIGHT = 900;
#endif

// forward declaration of functions
void CreateWindowWithSettings(sf::RenderWindow& window, GameSettings const& settings);


void Controls::LoadDefault()
{
	keys.clear();
	std::ifstream s("Resources/Data/default_controls.txt");
	std::string w, w2;

	while (s >> w) {
		if (w[0] == '"') {
			s >> w2;
			std::string w3 = w.substr(1, w.length()-2);
			keys.push_back({w3, sf::Keyboard::Key(stoi(w2))});
		}
	}

}

void Controls::LoadUserControls()
{
	keys.clear();
	std::ifstream s("Resources/Data/user_controls.txt");
	std::string w, w2;

	while (s >> w) {
		if (w[0] == '"') {
			s >> w2;
			std::string w3 = w.substr(1, w.length()-2);
			keys.push_back({w3, sf::Keyboard::Key(stoi(w2))});
		}
	}
}

void Controls::SaveUserControls()
{
	std::ofstream s("Resources/Data/user_controls.txt");
	for (auto k : keys) {
		s << '"' << k.first << '"' << ' ' << int(k.second) << endl;
	}
	cout << "Saved Controls" << endl;
}

Game::Game()
{
	CreateWindowWithSettings(window, game_settings);

	game_settings.controls.LoadUserControls();

	Item::InitRecipes();

	ButtonActionImpl* impl = new ButtonActionImpl(this, nullptr, nullptr);

	// game_state must be initialized before menu_state (because of button_action_impl)
	game_state = new GameState(*this);
	impl->game_state = game_state;
	game_state->Init(impl);

	menu_state = new MenuState(*this);
	impl->menu_state = menu_state;
	menu_state->Init(impl);
	menu_state->setActive(true);
	state_machine.PushState(State::MainMenu);

	console = new ConsoleNamespace::Console(*this, *menu_state, *game_state); // !!!!!!!!!!!!!! clears the console !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

}

Game::~Game()
{
	delete menu_state;
	delete game_state;
	delete console;
}

void Game::Start()
{
	sf::Clock clock;
	sf::Text fps_counter_text;
	fps_counter_text.setFont(ResourceManager::getFont(BASE_FONT_NAME));
	fps_counter_text.setCharacterSize(15);
	fps_counter_text.setFillColor(sf::Color::Black);
	fps_counter_text.setPosition(10, 10);
	fps_counter_text.setString("fps");

	sf::Clock refresh_clock;
	sf::Time refresh(sf::seconds(0.25f));
	int frames = 0;

	bool escape_pressed = false;

	sf::Clock dt_clock;
	float dt = 0;

	while (window.isOpen()) {
		if (refresh_clock.getElapsedTime() >= refresh) {
			refresh_clock.restart();
			float t = clock.restart().asMicroseconds() / 1000.f;
			fps_counter_text.setString(to_string(int(1000 / (t / frames))));
			frames = 0;
		}

		// Events
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) { Quit(); }

			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
				if (!escape_pressed) {
					quit_timer.restart();
					escape_pressed = true;
				}
			}
			else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape) {
				escape_pressed = false;
			}

			if (menu_state->getActive()) {
				if (menu_state->HandleEvents(event)) continue;
			}
			if (game_state->getActive()) {
				if (game_state->HandleEvent(event)) continue;
			}
			if (console->getActive()) {
				if (console->HandleEvent(event)) continue;
			}

			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Escape) {
					if (state_machine.getActiveState() == State::MainMenu) { Quit(); }
					else if (state_machine.getActiveState() == State::Game) { ChangeActiveState(State::PauseMenu, State::Game); }
					else { ReturnToLastState(); }
				}
				else if (event.key.code == sf::Keyboard::Space) {
					if (state_machine.getActiveState() == State::MainMenu) {
						game_state->StartNewGame("dev");
						ChangeActiveState(State::Game, State::MainMenu);
					}
				}
				else if (event.key.code == game_settings.controls.get("Console")) {
					console->setActive(true);
				}
				else {
					//cout << event.key.code << endl;
				}
			}
		}

		if (escape_pressed && quit_timer.getElapsedTime() >= QUICK_EXIT_TIME) {
			Quit();
		}

		// Updates
		if (menu_state->getActive()) { menu_state->Update(); }
		if (game_state->getActive()) { game_state->Update(dt); }
		if (console->getActive()) { console->Update(); }

		// Renders
		window.clear(sf::Color::White);

		if (menu_state->getActive()) { menu_state->Render(window); }
		if (game_state->getActive()) { game_state->Render(window); }
		window.setView(window.getDefaultView());
		if (console->getActive()) { console->Render(window); }

		window.setView(window.getDefaultView());
		if (show_fps_counter)
			window.draw(fps_counter_text);
		window.display();

		dt = dt_clock.restart().asMicroseconds() / 1000.f;
		++frames;
	}
}

void Game::Quit()
{
	window.close();
}

void Game::ChangeActiveState(State new_state, State old_state)
{
	// do we need to update the state machine stack?
	if (new_state != state_machine.getActiveState()) {
		if (old_state == state_machine.getActiveState()) {
			state_machine.PushState(new_state);
		}
	}

	if (new_state == State::Game) {
		game_state->setActive(true);
		menu_state->setActive(false);
	}
	else {
		game_state->setActive(false);
		menu_state->setActive(true);
		menu_state->setActiveState(new_state);
	}
}

void Game::ToggleFpsCounter()
{
	show_fps_counter = !show_fps_counter;
}

void CreateWindowWithSettings(sf::RenderWindow& window, GameSettings const& settings)
{
	sf::ContextSettings ctx_settings;
	ctx_settings.antialiasingLevel = settings.AALevel;

	sf::Uint32 style;
	sf::VideoMode video_mode;

	if (!settings.Fullscreen) {
		style = sf::Style::Close;
		video_mode = sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT);
	}
	else {
		style = sf::Style::Fullscreen;
		video_mode = sf::VideoMode::getDesktopMode();
	}

	window.create(video_mode, "Projet Intégrateur", style, ctx_settings);

	//window.setKeyRepeatEnabled(false);

	//window.setVerticalSyncEnabled(settings.VSync);
}

std::string getKeyString(sf::Keyboard::Key key)
{
	if (key >= 0 && key <= 25) {
		return string(1, char(65+int(key)));
	}
	if (key >= 26 && key <= 35) {
		return string(1, char(48+int(key)-26));
	}

	switch (key)
	{
	case sf::Keyboard::Unknown: return "???";
	case sf::Keyboard::Escape: return "Escape";
	case sf::Keyboard::LControl: return "LCtrl";
	case sf::Keyboard::LShift: return "LShift";
	case sf::Keyboard::LAlt: return "LAlt";
	case sf::Keyboard::RControl: return "RCtrl";
	case sf::Keyboard::RShift: return "RShift";
	case sf::Keyboard::RAlt: return "RAlt";
	case sf::Keyboard::SemiColon: return ";";
	case sf::Keyboard::Comma: return ",";
	case sf::Keyboard::Period: return ".";
	case sf::Keyboard::Quote: return "'";
	case sf::Keyboard::Slash: return "/";
	case sf::Keyboard::BackSlash: return "\\";
	case sf::Keyboard::Tilde: break;
	case sf::Keyboard::Equal: break;
	case sf::Keyboard::Dash: break;
	case sf::Keyboard::Space: return "Espace";
	case sf::Keyboard::Return: return "Entrée";
	case sf::Keyboard::BackSpace: return "Backspace";
	case sf::Keyboard::Tab: break;
	case sf::Keyboard::PageUp: break;
	case sf::Keyboard::PageDown: break;
	case sf::Keyboard::End: break;
	case sf::Keyboard::Home: break;
	case sf::Keyboard::Insert: break;
	case sf::Keyboard::Delete: break;
	case sf::Keyboard::Add: break;
	case sf::Keyboard::Subtract: break;
	case sf::Keyboard::Multiply: break;
	case sf::Keyboard::Divide: break;
	case sf::Keyboard::Left: return "Flèche gauche";
	case sf::Keyboard::Right: return "Flèche droite";
	case sf::Keyboard::Up: return "Flèche haute";
	case sf::Keyboard::Down: return "Flèche basse";

	case sf::Keyboard::Numpad0: break;
	case sf::Keyboard::Numpad1: break;
	case sf::Keyboard::Numpad2: break;
	case sf::Keyboard::Numpad3: break;
	case sf::Keyboard::Numpad4: break;
	case sf::Keyboard::Numpad5: break;
	case sf::Keyboard::Numpad6: break;
	case sf::Keyboard::Numpad7: break;
	case sf::Keyboard::Numpad8: break;
	case sf::Keyboard::Numpad9: break;

	case sf::Keyboard::F1: return "F1";
	case sf::Keyboard::F2: return "F2";
	case sf::Keyboard::F3: return "F3";
	case sf::Keyboard::F4: return "F4";
	case sf::Keyboard::F5: return "F5";
	case sf::Keyboard::F6: return "F6";
	case sf::Keyboard::F7: return "F7";
	case sf::Keyboard::F8: return "F8";
	case sf::Keyboard::F9: return "F9";
	case sf::Keyboard::F10:return "F10";
	case sf::Keyboard::F11:return "F11";
	case sf::Keyboard::F12:return "F12";
	default:
		return "??";
	}
	return std::string("????");
}

char getKeyChar(sf::Event::KeyEvent e)
{
	if (e.shift) {
		if (e.code >= 0 && e.code <= 25) {
			return char(65+int(e.code));
		}

		switch (e.code) {
		case sf::Keyboard::LBracket: return '[';
		case sf::Keyboard::RBracket: return ']';
		case sf::Keyboard::SemiColon: return ':';
		case sf::Keyboard::Comma: return '\'';
		case sf::Keyboard::Period: return '.';
		case sf::Keyboard::Quote: return '|';
		case sf::Keyboard::Slash: return 'É';
		case sf::Keyboard::BackSlash: return '>';
		case sf::Keyboard::Tilde: return '{';
		case sf::Keyboard::Equal: return '+';
		case sf::Keyboard::Dash: return '_';
		case sf::Keyboard::Space: return ' ';
		case sf::Keyboard::Num1: return '!';
		case sf::Keyboard::Num2: return '"';
		case sf::Keyboard::Num3: return '/';
		case sf::Keyboard::Num4: return '$';
		case sf::Keyboard::Num5: return '%';
		case sf::Keyboard::Num6: return '?';
		case sf::Keyboard::Num7: return '&';
		case sf::Keyboard::Num8: return '*';
		case sf::Keyboard::Num9: return '(';
		case sf::Keyboard::Num0: return ')';

		default: break;	
		}
	}
	else {
		if (e.code >= 0 && e.code <= 25) {
			return char(97+int(e.code));
		}

		if (!e.alt) {
			switch (e.code) {
			case sf::Keyboard::LBracket: return '[';
			case sf::Keyboard::RBracket: return ']';
			case sf::Keyboard::SemiColon: return ';';
			case sf::Keyboard::Comma: return ',';
			case sf::Keyboard::Period: return '.';
			case sf::Keyboard::Quote: return '#';
			case sf::Keyboard::Slash: return 'é';
			case sf::Keyboard::BackSlash: return '<';
			case sf::Keyboard::Tilde: return '{';
			case sf::Keyboard::Equal: return '=';
			case sf::Keyboard::Dash: return '-';
			case sf::Keyboard::Space: return ' ';
			default: break;
			}
		}

		if (e.alt) {
			switch (e.code) {
			case sf::Keyboard::LBracket: return '[';
			case sf::Keyboard::RBracket: return ']';
			case sf::Keyboard::SemiColon: return '~';
			case sf::Keyboard::Comma: return '¯';
			case sf::Keyboard::Period: return '.';
			case sf::Keyboard::Quote: return '\\';
			case sf::Keyboard::Slash: return 'É';
			case sf::Keyboard::BackSlash: return '}';
			case sf::Keyboard::Tilde: return '{';
			case sf::Keyboard::Equal: return '=';
			case sf::Keyboard::Dash: return '-';
			case sf::Keyboard::Space: return ' ';
			default: break;
			}
		}
	}

	if (e.code >= 26 && e.code <= 35) {
		return char(48+int(e.code)-26);
	}

	return 0;
}
