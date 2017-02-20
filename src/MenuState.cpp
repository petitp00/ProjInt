#include "MenuState.h"

#include "ResourceManager.h"

#include <iostream>
using namespace std;

// MENU PAGE
MenuPage::MenuPage(Game& game) :
	button_action_impl(game)
{
	tiny.setString("Projet Intégrateur");
	tiny.setFillColor(sf::Color::Black);
	tiny.setFont(ResourceManager::getFont(BASE_FONT_NAME));
	tiny.setPosition({ 100, 100 });
	tiny.setCharacterSize(FontSize::TINY);

	small = tiny;
	small.setCharacterSize(FontSize::SMALL);
	small.move({ 0, 80 });

	normal = small;
	normal.setCharacterSize(FontSize::NORMAL);
	normal.move({ 0, 80 });

	title = normal;
	title.setCharacterSize(FontSize::BIG);
	title.move({ 0, 80 });

	auto b = new TextButton("Butt on", { 800, 350 }, 16.f, FontSize::NORMAL);

	auto t = new Tooltip("Plus d'info. WEEEEEEEEEEEEEEW", sf::seconds(0.75f));
	b->setTooltip(t);

	gui_objects.push_back(b);
}

MenuPage::~MenuPage()
{
	for (auto o : gui_objects) {
		delete o;
	}
}

void MenuPage::Update()
{
}

void MenuPage::Render(sf::RenderTarget& target)
{
	target.draw(title);
	target.draw(tiny);
	target.draw(small);
	target.draw(normal);

	for (auto o : gui_objects) {
		o->Render(target);
	}
}

void MenuPage::MousePressedEvent(int mouse_x, int mouse_y)
{
	for (auto o : gui_objects) {
		if (o->getHovered()) {
			o->onClick(button_action_impl);
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
	target.clear(sf::Color::Magenta);

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
