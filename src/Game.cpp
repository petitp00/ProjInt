#include "Game.h"

#include <SFML/Window/Event.hpp>

#include "GameState.h"
#include "MenuState.h"

#include <iostream>
using namespace std;

// forward declaration of functions
void CreateWindowWithSettings(sf::RenderWindow& window, GameSettings const& settings);


Game::Game()
{
	CreateWindowWithSettings(window, game_settings);

	menu_state = new MenuState(*this);
	menu_state->setActive(true);
	state_machine.PushState(State::MainMenu);

	game_state = new GameState();
}

Game::~Game()
{
	delete menu_state;
	delete game_state;
}

/*
	The game is locked at 60 fps for now. May or may not change later.
*/
void Game::Start()
{
	while (window.isOpen()) {

		// Events
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Escape) {
					if (state_machine.getActiveState() == State::MainMenu) {
						window.close();
					}
					else {
						auto s = state_machine.PopState();
						ChangeActiveState(state_machine.getActiveState(), s);
					}

				}
				else if (event.key.code == sf::Keyboard::Space) {
					ChangeActiveState(State::Game, state_machine.getActiveState());
				}

				if (menu_state->getActive()) {
					menu_state->KeyPressedEvent(event.key.code);
				}
			}
			else if (event.type == sf::Event::MouseButtonPressed) {
				if (menu_state->getActive()) {
					menu_state->MousePressedEvent(event.mouseButton.button, event.mouseButton.x, event.mouseButton.y);
				}
				else if (game_state->getActive()) {
					game_state->MousePressedEvent(event.mouseButton.button, event.mouseButton.x, event.mouseButton.y);
				}
			}
			else if (event.type == sf::Event::MouseMoved) {
				if (menu_state->getActive()) {
					menu_state->MouseMovedEvent(event.mouseMove.x, event.mouseMove.y);
				}

			}
		}

		// Updates
		if (menu_state->getActive()) { menu_state->Update(); }
		if (game_state->getActive()) { game_state->Update(); }


		// Renders
		window.clear(sf::Color::White);

		if (menu_state->getActive()) { menu_state->Render(window); }
		if (game_state->getActive()) { game_state->Render(window); }

		window.display();
	}
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
	}
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

	window.setVerticalSyncEnabled(settings.VSync);
}
