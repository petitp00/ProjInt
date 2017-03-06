#pragma once

#include "Game.h"

#include "GUIObjects.h"

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

class MenuPage
{
public:
	MenuPage();
	virtual ~MenuPage();

	void Clear();

	virtual void Update();
	virtual void Render(sf::RenderTarget& target);

	bool MousePressedEvent(int mouse_x, int mouse_y);
	bool MouseReleasedEvent(int mouse_x, int mouse_y);
	bool MouseMovedEvent(int mouse_x, int mouse_y);
	bool MouseWheelScrolledEvent(float delta);
	bool KeyPressedEvent(sf::Event::KeyEvent e);

	void AddGUIObject(GUIObject* obj) { gui_objects.push_back(obj); }

protected:

	sf::RenderTexture tooltip_render_target; // on top
	sf::Sprite tooltip_render_target_sprite;

	std::vector<GUIObject*> gui_objects;
};


class MenuState
{
public:
	MenuState(Game& game);
	~MenuState();

	void Update();
	void Render(sf::RenderTarget& target);

	bool HandleEvents(sf::Event const& event);

	bool getActive() const { return active; }
	void setActive(bool active) { this->active = active; }

	void setActiveState(State active_state) {
		this->active_state = active_state;
		if (active_state == State::MainMenu) active_page = &main_menu;
		if (active_state == State::NewGameMenu) active_page = &new_game_menu;
		else if (active_state == State::InfoMenu) active_page = &info_menu;
		else if (active_state == State::OptionsMenu) active_page = &options_menu;
		else if (active_state == State::AudioOptionsMenu) active_page = &audio_menu;
		else if (active_state == State::ControlsOptionsMenu) active_page = &controls_menu;
		else if (active_state == State::PauseMenu) active_page = &pause_menu;
	}

	void ResetControls(Controls& controls) {
		controls_menu.Clear();
		InitControlsMenu(controls);
	}

private:
	ButtonActionImpl button_action_impl;

	bool active = false;
	State active_state = State::MainMenu;
	MenuPage* active_page = nullptr;

	void InitMainMenu();
	MenuPage main_menu;

	void InitNewGameMenu();
	MenuPage new_game_menu;

	void InitInfoMenu();
	MenuPage info_menu;

	void InitOptionsMenu();
	MenuPage options_menu;

	void InitAudioMenu();
	MenuPage audio_menu;

	void InitControlsMenu(Controls& controls);
	MenuPage controls_menu;

	void InitPauseMenu();
	MenuPage pause_menu;
};
