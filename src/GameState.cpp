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

void GameState::StartNewGame(std::string const & name)
{
	world.CreateAndSaveWorld(name);
}

void GameState::LoadGame(std::string const & name)
{
	world.LoadWorld(name);
}
