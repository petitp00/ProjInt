#include "GameState.h"

#include "Console.h"
#include "rng.h"
#include "ResourceManager.h"

#include <iostream>
using namespace std;

static vec2 mouse_pos = {-1, -1};
static vec2 mouse_pos_in_world = {-1, -1};

float interact_bar_max_w = 40.f;
float interact_bar_h = 6.f;
sf::Time interact_time = sf::seconds(0.7f);

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
	tool_obj.Init(&inventory, this);
	world.Init(&inventory, &inv_butt, this);

	interact_bar1.setFillColor(sf::Color::Yellow);
	interact_bar2.setFillColor(sf::Color(255, 150, 0)); // orange
	interact_bar2.setSize(vec2(interact_bar_max_w, interact_bar_h));
	equipped_tool_sprite.setTexture(ResourceManager::getTexture(Item::texture_map_file));
	collecting_sprite.setTexture(ResourceManager::getTexture(Item::texture_map_file));

	gui_status_bars.Init();
	status_values.Init(&gui_status_bars);
}

void GameState::Update(float dt)
{
	// Update of use tool and collect
	auto p = mouse_pos - vec2(interact_bar_max_w / 2.f, - 14);
	interact_bar1.setPosition(p);
	interact_bar2.setPosition(p);

	Item::Tool* t = nullptr;
	if (equipped_tool != -1) {
		t = Item::Manager::getTool(equipped_tool);
	}

	auto old_cut = can_use_tool;
	Item::ItemType icon_type;
	can_use_tool = world.getCanUseTool(equipped_tool, icon_type);

	if (old_cut && !can_use_tool) {
		using_tool = false;
	}
	else if (!old_cut && can_use_tool) {
		if (equipped_tool == -1) {
			equipped_tool_sprite.setTextureRect(Item::getItemTextureRect(icon_type));
		}
		else {
			int ts = int(Item::items_texture_size);
			equipped_tool_sprite.setTextureRect(sf::IntRect(t->pos_in_texture_map*ts, vec2i(ts, ts)));
		}
	}

	if (using_tool && can_use_tool) {
		sf::Time use_speed = (equipped_tool != -1) ? t->use_speed : sf::seconds(2);
		if (interact_clock.getElapsedTime() >= use_speed) {
			interact_clock.restart();
			world.UseEquippedToolAt();
			inventory.UseEquippedTool(); // must be called after world.UseEquippedToolAt for bowls
			EquipTool(equipped_tool); // to update bowl's sprite
		}
		interact_bar1.setSize(vec2(interact_bar_max_w * (1 - interact_clock.getElapsedTime().asSeconds() / use_speed.asSeconds()), interact_bar_h));
	}


	auto old_coll = can_collect;
	can_collect = world.getCanCollect(collect_type);
	if (old_coll && !can_collect) {
		collecting = false;
	}
	if (!old_coll && can_collect) {
		auto text_rect = Item::getItemTextureRect(collect_type);
		collecting_sprite.setTextureRect(sf::IntRect(text_rect));
	}
	if (collecting && can_collect) {
		if (interact_clock.getElapsedTime() >= interact_time) {
			interact_clock.restart();
			world.Collect();
		}
		interact_bar1.setSize(vec2(interact_bar_max_w * (1 - interact_clock.getElapsedTime().asSeconds() / interact_time.asSeconds()), interact_bar_h));
	}

	if (collecting) can_use_tool = false;
	if (using_tool) can_collect = false;

	if (can_collect && can_use_tool) {
		collecting_sprite.setPosition(mouse_pos - ets_size/2.f - vec2(ets_size.x * 1.f, 0));
		equipped_tool_sprite.setPosition(mouse_pos - ets_size/2.f + vec2(ets_size.x * 1.f, 0));
	}
	else {
		collecting_sprite.setPosition(mouse_pos - ets_size/2.f);
		equipped_tool_sprite.setPosition(mouse_pos - ets_size/2.f);
	}

	// Updates

	if (!game.getConsole().getActive())
		world.Update(dt, mouse_pos_in_world);
	inventory.Update();
	UpdateGUIHoverInfo();
	hover_info.Update();
	action_info.Update();
	gui_status_bars.Update(mouse_pos);
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
	if (can_collect) {
		target.draw(collecting_sprite);
	}
	if (using_tool || collecting) {
		target.draw(interact_bar2);
		target.draw(interact_bar1);
	}

	if (!inventory.getActive()) {
		hover_info.Render(target);
		action_info.Render(target);
	}
	gui_status_bars.Render(target);

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
					collecting = false;
					can_collect = false;
					interact_clock.restart();
				}
			}
			else if (event.mouseButton.button == sf::Mouse::Left) {
				if (can_collect) {
					collecting = true;
					using_tool = false;
					can_use_tool = false;
					interact_clock.restart();
				}
			}
		}
		else if (event.type == sf::Event::MouseButtonReleased) {
			if (event.mouseButton.button == sf::Mouse::Right) {
				using_tool = false;
			}
			if (event.mouseButton.button == sf::Mouse::Left) {
				collecting = false;
			}
		}
		else if (event.type == sf::Event::MouseMoved) {
			//UpdateGUIHoverInfo();
		}

		inv_butt.HandleEvent(event);
		tool_obj.HandleEvent(event);
	}
	return world.HandleEvent(event);
}

void GameState::UpdateGUIHoverInfo()
{
	static Entity* prev_e = nullptr;
	static GroundTile* prev_t = nullptr;

	auto e = world.FindEntityClicked(mouse_pos_in_world);
	if (e && e != prev_e) {
		hover_info.setString(e->getHoverInfo());
		prev_e = e;
		prev_t = nullptr;
	}
	else if (e && e == prev_e && e->getHoverInfoChanged()) {
		hover_info.setString(e->getHoverInfo());
		e->setHoverInfoChanged(false);
	}
	else if (!e) {
		auto t = world.getGroundTileHovered(mouse_pos_in_world);
		if (t) {
			hover_info.setString(t->getName() + '\n');
			prev_t = t;
		}
		else if (prev_e) {
			hover_info.setString("");
		}

		prev_e = nullptr;
	}
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
	inventory.UnequipTool();
	tool_obj.setTool(-1);
	can_use_tool = false;
	using_tool = false;
}

void GameState::DoAction()
{
	status_values.set(StatusType::energy, status_values.get(StatusType::energy) - 15);
	status_values.set(StatusType::hunger, status_values.get(StatusType::hunger) - 8);
	status_values.set(StatusType::thirst, status_values.get(StatusType::thirst) - 8);
}

void GameState::Eat()
{
	status_values.set(StatusType::hunger, status_values.get(StatusType::hunger) + 18);
}

void GameState::Drink()
{
	status_values.set(StatusType::thirst, status_values.get(StatusType::thirst) + 18);
}

void GameState::Sleep()
{
	status_values.set(StatusType::energy, status_values.get(StatusType::energy) + 100);
}
