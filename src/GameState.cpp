#include "GameState.h"

#include "rng.h"
#include "ResourceManager.h"

#include <iostream>
using namespace std;

GameState::GameState(Controls* controls) :
	world(*this, controls)
{
}

GameState::~GameState()
{
}

void GameState::Update(float dt)
{
	world.Update(dt);
}

void GameState::Render(sf::RenderTarget & target)
{
	target.clear(sf::Color::White);

	world.Render(target);
}

bool GameState::HandleEvent(sf::Event const & event)
{
	return world.HandleEvent(event);
}
