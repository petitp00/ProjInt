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

// MENU PAGE
MenuPage::MenuPage(Game& game) :
	button_action_impl(game)
{
	tooltip_render_target.create(WINDOW_WIDTH, WINDOW_HEIGHT);
	tooltip_render_target.setSmooth(true);
	tooltip_render_target_sprite.setTexture(tooltip_render_target.getTexture());

	Init();
}

MenuPage::~MenuPage()
{
	for (auto o : gui_objects) {
		delete o;
	}
}

void MenuPage::Init()
{
	title.setString("Projet Intégrateur");
	title.setFillColor(sf::Color::Black);
	title.setFont(ResourceManager::getFont(BASE_FONT_NAME));
	title.setPosition({ 100, 80 });
	title.setCharacterSize(FontSize::BIG);

	float button_width = title.getLocalBounds().width;

	auto play_button = new TextButton("Jouer", { 100, 250 }, button_width);
	play_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(start_game), &button_action_impl);
	play_button->setTooltip(new Tooltip("Commencer une partie", sf::seconds(0.55f)));
	gui_objects.push_back(play_button);

	auto load_button = new TextButton("Charger une partie", { 100, 350 }, button_width);
	//load_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(test), &button_action_impl);
	load_button->setTooltip(new Tooltip("Charger une partie sauvegardée", sf::seconds(0.55f)));
	gui_objects.push_back(load_button);

	auto options_button = new TextButton("Options", { 100, 450 }, button_width/2.f - 20.f);
	options_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(start_game), &button_action_impl);
	options_button->setTooltip(new Tooltip("Accéder aux options", sf::seconds(0.55f)));
	gui_objects.push_back(options_button);

	auto info_button = new TextButton("Informations", { 100 + button_width/2.f + 20.f, 450 }, button_width/2.f - 20.f);
	info_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(start_game), &button_action_impl);
	info_button->setTooltip(new Tooltip("Informations sur le projet et ses créateurs", sf::seconds(0.55f)));
	gui_objects.push_back(info_button);

	auto quit_button = new TextButton("Quitter", { 100, 550 }, button_width);
	quit_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(quit_game), &button_action_impl);
	quit_button->setTooltip(new Tooltip("Quitter le jeu", sf::seconds(0.55f)));
	gui_objects.push_back(quit_button);
}

void MenuPage::Update()
{
	for (auto o : gui_objects) {
		o->Update();
	}
}

void MenuPage::Render(sf::RenderTarget& target)
{
	tooltip_render_target.clear(sf::Color::Transparent);

	target.draw(title);

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
MenuState::MenuState(Game& game) : main_menu(game)
{
}

MenuState::~MenuState()
{
}

void MenuState::Update()
{
	if (active) {
		main_menu.Update();
	}
}

void MenuState::Render(sf::RenderTarget & target)
{
	target.clear(sf::Color(183, 241, 244));

	main_menu.Render(target);
}

void MenuState::KeyPressedEvent(sf::Keyboard::Key key)
{
}

void MenuState::MousePressedEvent(sf::Mouse::Button button, int mouse_x, int mouse_y)
{
	main_menu.MousePressedEvent(mouse_x, mouse_y);
}

void MenuState::MouseMovedEvent(int mouse_x, int mouse_y)
{
	main_menu.MouseMovedEvent(mouse_x, mouse_y);
}
