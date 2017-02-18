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
