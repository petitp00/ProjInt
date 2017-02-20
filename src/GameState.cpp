#include "GameState.h"

#include "ResourceManager.h"

#include <iostream>
using namespace std;

GameState::GameState()
{
	wew.setCharacterSize(FontSize::BIG * 2);
	wew.setFont(ResourceManager::getFont(BASE_FONT_NAME));
	wew.setString("WEEEEEEEEEEEW");
	wew.setFillColor(sf::Color::White);
	wew.setOrigin(wew.getLocalBounds().width / 2.f, wew.getLocalBounds().height / 2.f);
	wew.setPosition(1280 / 2.f, 720 / 2.f);
}

GameState::~GameState()
{
}

void GameState::Update()
{
	auto p = wew.getPosition();
	float rx, ry;

	rx = (rand() % 15 - 7.5f);
	ry = (rand() % 15 - 7.5f);

	if (active) {
		wew.setPosition(wew.getPosition().x + rx, wew.getPosition().y + ry);
	}
}

void GameState::Render(sf::RenderTarget & target)
{
	target.clear(sf::Color::Red);

	target.draw(wew);
}

void GameState::KeyPressedEvent(sf::Keyboard::Key key)
{
}

void GameState::MousePressedEvent(sf::Mouse::Button button, int mouse_x, int mouse_y)
{
}
