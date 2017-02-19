#include "GameState.h"

GameState::GameState()
{
}

GameState::~GameState()
{
}

void GameState::Update()
{
	if (active) {

	}
}

void GameState::Render(sf::RenderTarget & target)
{
	target.clear(sf::Color::Red);
}

void GameState::KeyPressedEvent(sf::Keyboard::Key key)
{
}

void GameState::MousePressedEvent(sf::Mouse::Button button, int mouse_x, int mouse_y)
{
}
