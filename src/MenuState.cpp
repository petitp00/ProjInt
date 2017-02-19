#include "MenuState.h"

#include <iostream>
using namespace std;

MenuState::MenuState() 
{
}

MenuState::~MenuState()
{
}

void MenuState::Update()
{
	if (active) {

	}
}

void MenuState::Render(sf::RenderTarget & target)
{
	target.clear(sf::Color::Magenta);
}

void MenuState::KeyPressedEvent(sf::Keyboard::Key key)
{
}

void MenuState::MousePressedEvent(sf::Mouse::Button button, int mouse_x, int mouse_y)
{
}
