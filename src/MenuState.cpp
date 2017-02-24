#include "MenuState.h"

#include "ResourceManager.h"

#include <iostream>
using namespace std;


static void start_game(ButtonActionImpl* impl) {
	impl->game.ChangeActiveState(State::Game, State::MainMenu);
}

static void quit_game(ButtonActionImpl* impl) {
	impl->game.Quit();
}

static void go_to_info_menu(ButtonActionImpl* impl) {
	impl->game.ChangeActiveState(State::InfoMenu, State::MainMenu);
}

static void return_to_last_state(ButtonActionImpl* impl) {
	impl->game.ReturnToLastState();
}

// MENU PAGE
MenuPage::MenuPage()
{
	tooltip_render_target.create(WINDOW_WIDTH, WINDOW_HEIGHT);
	tooltip_render_target.setSmooth(true);
	tooltip_render_target_sprite.setTexture(tooltip_render_target.getTexture());
}

MenuPage::~MenuPage() { for (auto o : gui_objects) { delete o; } }

void MenuPage::Update() { for (auto o : gui_objects) { o->Update(); } }

void MenuPage::Render(sf::RenderTarget& target)
{
	tooltip_render_target.clear(sf::Color::Transparent);

	for (auto o : gui_objects) {
		o->Render(target, tooltip_render_target);
	}

	tooltip_render_target.display();
	target.draw(tooltip_render_target_sprite);
}

void MenuPage::MousePressedEvent(int mouse_x, int mouse_y)
{
	for (auto o : gui_objects) {
		if (o->getHovered()) {
			o->onClick();
		}
	}
}

void MenuPage::MouseMovedEvent(int mouse_x, int mouse_y)
{
	sf::Vector2i mouse(mouse_x, mouse_y);
	for (auto o : gui_objects) {
		if (o->isMouseIn(mouse)) {
			if (!o->getHovered()) {
				o->onHoverIn(mouse);
			}
			else {
				o->UpdateHoveredMousePos(mouse);
			}
		}
		else if (o->getHovered()) {
			o->onHoverOut();
		}
	}
}

// MENU STATE
MenuState::MenuState(Game& game) :
	button_action_impl(game)
{
	InitMainMenu();
	InitInfoMenu();
}

MenuState::~MenuState()
{
}

void MenuState::Update()
{
	if (active) {
		if (active_state == State::MainMenu)
			main_menu.Update();
		else if (active_state == State::InfoMenu)
			info_menu.Update();
	}
}

void MenuState::Render(sf::RenderTarget & target)
{
	target.clear(sf::Color(183, 241, 244));

	if (active_state == State::MainMenu)
		main_menu.Render(target);
	else if (active_state == State::InfoMenu)
		info_menu.Render(target);
}

void MenuState::KeyPressedEvent(sf::Keyboard::Key key)
{
}

void MenuState::MousePressedEvent(sf::Mouse::Button button, int mouse_x, int mouse_y)
{
	if (active_state == State::MainMenu)
		main_menu.MousePressedEvent(mouse_x, mouse_y);
	else if (active_state == State::InfoMenu)
		info_menu.MousePressedEvent(mouse_x, mouse_y);
}

void MenuState::MouseMovedEvent(int mouse_x, int mouse_y)
{
	if (active_state == State::MainMenu)
		main_menu.MouseMovedEvent(mouse_x, mouse_y);
	else if (active_state == State::InfoMenu)
		info_menu.MouseMovedEvent(mouse_x, mouse_y);
}

void MenuState::InitMainMenu()
{
	auto title = new TextBox("Projet Intégrateur", { 100, 80 }, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	main_menu.AddGUIObject(title);

	float button_width = title->getSize().x;

	auto play_button = new TextButton("Jouer", { 100, 250 }, button_width);
	play_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(start_game), &button_action_impl);
	play_button->setTooltip(new Tooltip("Commencer une partie", sf::seconds(0.55f)));
	main_menu.AddGUIObject(play_button);

	auto load_button = new TextButton("Charger une partie", { 100, 350 }, button_width);
	load_button->setTooltip(new Tooltip("Charger une partie sauvegardée", sf::seconds(0.55f)));
	main_menu.AddGUIObject(load_button);

	auto options_button = new TextButton("Options", { 100, 450 }, button_width / 2.f - 20.f);
	options_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(start_game), &button_action_impl);
	options_button->setTooltip(new Tooltip("Accéder aux options", sf::seconds(0.55f)));
	main_menu.AddGUIObject(options_button);

	auto info_button = new TextButton("Informations", { 100 + button_width / 2.f + 20.f, 450 }, button_width / 2.f - 20.f);
	info_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_info_menu), &button_action_impl);
	info_button->setTooltip(new Tooltip("Informations sur le projet et ses créateurs", sf::seconds(0.55f)));
	main_menu.AddGUIObject(info_button);

	auto quit_button = new TextButton("Quitter", { 100, 550 }, button_width);
	quit_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(quit_game), &button_action_impl);
	quit_button->setTooltip(new Tooltip("Quitter le jeu", sf::seconds(0.55f)));
	main_menu.AddGUIObject(quit_button);
}

void MenuState::InitInfoMenu()
{
	auto title = new TextBox("Informations", { 100, 80 }, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	info_menu.AddGUIObject(title);

	auto return_button = new TextButton("Retour", { float(WINDOW_WIDTH - 220), float(WINDOW_HEIGHT - 100) }, 0);
	return_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), &button_action_impl);
	return_button->setTooltip(new Tooltip("Menu principal", sf::seconds(0.55f)));
	info_menu.AddGUIObject(return_button);
}
