#include "GameState.h"

#include "Console.h"
#include "rng.h"
#include "ResourceManager.h"

#include <iostream>
using namespace std;

static vec2 mouse_pos = {-1, -1};
static vec2 mouse_pos_in_world = {-1, -1};

float use_tool_bar_max_w = 40.f;
float use_tool_bar_h = 6.f;

vec2 ets_size = vec2(Item::items_texture_size, Item::items_texture_size);

GameState::GameState(Game& game) :
	game(game),
	inventory(&game.getControls()),
	world(&game.getControls())
{
}

GameState::~GameState()
{
}

void GameState::Init(ButtonActionImpl * button_action_impl)
{
	inventory.Init(button_action_impl);
	inv_butt.Init(&inventory);
	tool_obj.Init(&inventory);
	world.Init(&inventory, &inv_butt, this);

	using_tool_bar1.setFillColor(sf::Color::Yellow);
	using_tool_bar2.setFillColor(sf::Color(255, 150, 0)); // orange
	using_tool_bar2.setSize(vec2(use_tool_bar_max_w, use_tool_bar_h));
	equipped_tool_sprite.setTexture(ResourceManager::getTexture(Item::texture_map_file));
}

void GameState::Update(float dt)
{
	auto p = mouse_pos - vec2(use_tool_bar_max_w / 2.f, - 14);
	using_tool_bar1.setPosition(p);
	using_tool_bar2.setPosition(p);

	if (equipped_tool != -1) {
		auto t = Item::Manager::getTool(equipped_tool);
		auto old_cut = can_use_tool;
		can_use_tool = world.getCanUseTool(t->name);
		if (old_cut && !can_use_tool) {
			using_tool = false;
		}

		if (using_tool && can_use_tool) {
			if (using_tool_clock.getElapsedTime() >= t->use_speed) {
				using_tool_clock.restart();
				inventory.UseEquippedTool();
				world.UseEquippedToolAt(mouse_pos_in_world);
			}
			using_tool_bar1.setSize(vec2(use_tool_bar_max_w * (1 - using_tool_clock.getElapsedTime().asSeconds() / t->use_speed.asSeconds()), use_tool_bar_h));
		}

		equipped_tool_sprite.setPosition(mouse_pos - ets_size/2.f);
	}

	if (!game.getConsole().getActive())
		world.Update(dt, mouse_pos_in_world);
	inventory.Update();
}

void GameState::Render(sf::RenderTarget & target)
{
	target.clear(sf::Color::White);

	world.Render(target);

	target.setView(target.getDefaultView());
	inv_butt.Render(target);
	tool_obj.Render(target);
	if (can_use_tool) {
		target.draw(equipped_tool_sprite);
	}
	if (using_tool) {
		target.draw(using_tool_bar2);
		target.draw(using_tool_bar1);
	}

	inventory.Render(target);

}

bool GameState::HandleEvent(sf::Event const & event)
{
	mouse_pos_in_world = game.getWindow().mapPixelToCoords(sf::Mouse::getPosition(game.getWindow()), world.getGameView());
	mouse_pos = vec2(sf::Mouse::getPosition(game.getWindow()));

	if (game.getConsole().getActive()) {
		if (event.type == sf::Event::MouseButtonPressed) {
			if (event.mouseButton.button == sf::Mouse::Middle) {
				auto p = game.getWindow().mapPixelToCoords(sf::Mouse::getPosition(game.getWindow()), world.getGameView());
				auto e = world.FindEntityClicked(p);
				if (e) {
					game.getConsole().PrintInfo("Entity clicked");
					game.getConsole().PrintInfo("ID: " + to_string(e->getId()));
					game.getConsole().PrintInfo("Type: " + to_string(e->getType()) + "   (" + getEntityTypeString(e->getType()) + ")");
				}
			}
		}
	}
	else if (inventory.getActive()) {
		if (event.type == sf::Event::MouseButtonPressed) {
			if (event.mouseButton.button == sf::Mouse::Left) {
				if (!inventory.IsMouseIn(sf::Mouse::getPosition(game.getWindow()))) {
					//inventory.setActive(false);
				}
			}
		}

		if (inventory.HandleEvents(event)) return true;
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
			inventory.setActive(false);
			return true;
		}
	}
	else {
		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == game.getControls().get("Inventaire")) {
				inventory.setActive(true);
				return true;
			}
		}
		if (event.type == sf::Event::MouseButtonPressed) {
			if (event.mouseButton.button == sf::Mouse::Right) {
				if (can_use_tool) {
					using_tool = true;
					using_tool_clock.restart();
				}
			}
		}
		else if (event.type == sf::Event::MouseButtonReleased) {
			if (event.mouseButton.button == sf::Mouse::Right) {
				using_tool = false;
			}
		}

		inv_butt.HandleEvent(event);
		tool_obj.HandleEvent(event);
	}
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

void GameState::EquipTool(int id)
{
	equipped_tool = id;
	tool_obj.setTool(id);

	if (id != -1) {
		auto t = Item::Manager::getTool(id);
		int ts = int(Item::items_texture_size);
		equipped_tool_sprite.setTextureRect(sf::IntRect(t->pos_in_texture_map*ts, vec2i(ts, ts)));
	}
}

void GameState::UnequipTool()
{
	equipped_tool = -1;
	tool_obj.setTool(-1);
	can_use_tool = false;
	using_tool = false;
}
