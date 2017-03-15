#include "Editor.h"
#include "../Console.h"
#include "../ResourceManager.h"

#include <iostream>

using namespace EditorMode;
using namespace std;

Editor::Editor()
{
	game_view_size ={float(WINDOW_W), float(WINDOW_H)};
	minimap_view_size ={float(WORLD_W), float(WORLD_H)};
	game_view.reset(sf::FloatRect({0, 0}, game_view_size));
	minimap_view.reset(sf::FloatRect({0,0}, minimap_view_size));
	//game_view.setViewport(sf::FloatRect(0, 0, 0.5f, 0.5f));
	float sz = 200.f;
	float vpw = sz/float(WINDOW_W);
	float vph = sz/float(WINDOW_H);

	//minimap_view.setViewport(sf::FloatRect(1.f-vpw, 0.f, vpw, vph));
	game_view_minimap_shape.setFillColor(sf::Color::Transparent);
	game_view_minimap_shape.setOutlineColor(sf::Color::Green);
	game_view_minimap_shape.setOutlineThickness(2);

	controls = new Controls;
	controls->LoadDefault(); 
	world.controls = controls;

	console = new ConsoleNamespace::Console(*this);
	window.create(sf::VideoMode(WINDOW_W, WINDOW_H), "Editor", sf::Style::Close);
}

Editor::~Editor()
{
	delete console;
}

void Editor::Start()
{
	sf::Clock clock;
	sf::Clock refresh_clock;
	sf::Time refresh(sf::seconds(0.25f));
	int frames = 0;
	sf::Clock dt_clock;
	float dt = 0;

	bool escape_pressed = false;

	sf::Vector2f mpos;
	ground_edit_info = make_info("ground_edit", false);
	gui_infos.push_back(ground_edit_info);
	auto fps_info = make_info("fps");
	gui_infos.push_back(fps_info);
	gui_infos.push_back(make_info("mouse (world)"));
	auto gui_info_id = make_info("id", false);
	gui_infos.push_back(gui_info_id);
	auto gui_info_zoom = make_info("zoom", false);
	gui_infos.push_back(gui_info_zoom);

	Entity* selected_entity = nullptr;
	sf::Vector2f select_click_entity_offset;
	auto selected_entity_pos_label = make_info("");

	sf::RenderTexture minimap_texture;
	minimap_texture.create(200, 200);
	minimap_texture.setView(minimap_view);
	sf::Sprite minimap_sprite(minimap_texture.getTexture());
	minimap_sprite.setColor(sf::Color(255, 255, 255, 128));
	minimap_sprite.setPosition(WINDOW_W-204, 4);
	sf::RectangleShape minimap_border_shape(sf::Vector2f(200, 200));
	minimap_border_shape.setFillColor(sf::Color::Transparent);
	minimap_border_shape.setOutlineColor(sf::Color::Black);
	minimap_border_shape.setOutlineThickness(3);
	minimap_border_shape.setPosition(WINDOW_W - 204, 4);

	bool middle_pressed = false;
	bool left_pressed = false;
	bool minimap_drag = false;
	sf::Vector2f drag_mouse_pos;
	sf::Vector2f last_ground_edit{-1.f, -1.f};
	
	while (window.isOpen()) {
		if (refresh_clock.getElapsedTime() >= refresh) {
			refresh_clock.restart();
			float t = clock.restart().asMicroseconds() / 1000.f;
			fps_info->setVal(to_string(int(1000 / (t / frames))));

			frames = 0;
		}

		// Events
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) { Quit(); }

			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Escape) {
					if (!escape_pressed) {
						quit_timer.restart();
						escape_pressed = true;
					}
				}
				else if (event.key.code == sf::Keyboard::F1) {
					console->setActive(true);
				}
			}
			else if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Escape) {
				escape_pressed = false;
			}

			if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Middle) {
					auto p = window.mapPixelToCoords(sf::Mouse::getPosition(window), game_view);
					auto e = world.FindEntityClicked(p);
					if (e) {
						console->PrintInfo("Entity clicked");
						console->PrintInfo("ID: " + to_string(e->getId()));
						console->PrintInfo("Type: " + to_string(e->getType()) + "   (" + getEntityTypeString(e->getType()) + ")");
						console->PrintInfo("Pos: {" + to_string(e->getPos().x) + ", " + to_string(e->getPos().y) + "}");
					}
					
					middle_pressed = true;
					drag_mouse_pos = p;
				}
				else if (event.mouseButton.button == sf::Mouse::Left) {
					left_pressed = true;
					auto mp = sf::Vector2f(sf::Mouse::getPosition(window));
					sf::FloatRect rect(minimap_border_shape.getPosition(), minimap_border_shape.getSize());
					if (rect.contains(mp)) {
						minimap_drag = true;
						drag_mouse_pos = mp - minimap_border_shape.getPosition();
					}

					auto p = window.mapPixelToCoords(sf::Mouse::getPosition(window), game_view);

					if (ground_edit_mode) {
						world.ground.setTileClicked(p, GroundType(ground_type));
					}
					else {
						auto e = world.FindEntityClicked(p);
						if (e) {
							select_click_entity_offset = p - e->getPos();
							selected_entity = e;
							gui_info_id->active = true;
							gui_info_id->setVal(to_string(selected_entity->getId()));
							selected_entity_pos_label->text_obj.setPosition(
								sf::Vector2f(window.mapCoordsToPixel(selected_entity->getPos() + sf::Vector2f(0, selected_entity->getSize().y + 10.f), game_view)));
						}
					}
				}
			}
			if (event.type == sf::Event::MouseButtonReleased) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					minimap_drag = false;
					left_pressed = false;
					if (selected_entity) selected_entity = nullptr;
				}
				else if (event.mouseButton.button == sf::Mouse::Middle) {
					middle_pressed = false;
				}
			}

			if (event.type == sf::Event::MouseWheelScrolled) {
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
					auto delta = event.mouseWheelScroll.delta;
					float factor = 0.15f;
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) factor = 0.45f;
					if (delta < 0) { game_view_zoom += factor; }
					else if (delta > 0) { game_view_zoom -= factor; }
					gui_info_zoom->setVal(to_string(game_view_zoom));
					game_view.setSize(game_view_size * game_view_zoom);
					UpdateGameViewMinimapShape();
				}
				else if (ground_edit_mode) {
					auto delta = event.mouseWheelScroll.delta;
					if (delta > 0) {
						if (ground_type != ground_type_max) ++ground_type;
						else ground_type = 0;
					}
					else {
						if (ground_type != 0) --ground_type;
						else ground_type = ground_type_max;
					}
					ground_edit_info->setVal(getGroundTypeString(GroundType(ground_type)));
				}
			}

			if (event.type == sf::Event::MouseMoved) {
				mpos = window.mapPixelToCoords(sf::Mouse::getPosition(window), game_view);
				if (selected_entity) {
					selected_entity->setPos(mpos - select_click_entity_offset);
					selected_entity_pos_label->setVal("{"+ to_string(int(selected_entity->getPos().x)) + ", " + to_string(int(selected_entity->getPos().y)) + "}");
					selected_entity_pos_label->text_obj.setPosition(
						sf::Vector2f(window.mapCoordsToPixel(selected_entity->getPos() + sf::Vector2f(0, selected_entity->getSize().y + 10.f),
						game_view)));

				}
				else {
					if (left_pressed && ground_edit_mode) {
						sf::Vector2f pi;
						pi.x = int(mpos.x/Ground::getVisualTileSize());
						pi.y = int(mpos.y/Ground::getVisualTileSize());

						if (pi != last_ground_edit) {
							last_ground_edit = pi;
							world.ground.setTileClicked(mpos, GroundType(ground_type));
						}
					}
					else {
						auto e = world.FindEntityClicked(mpos);
						if (e) {
							gui_info_id->active = true;
							gui_info_id->setVal(to_string(e->getId()));
						}
						else {
							gui_info_id->active = false;
						}
					}
				}

				if (middle_pressed) {
					game_view.setCenter(game_view.getCenter() + drag_mouse_pos - mpos);
					UpdateGameViewMinimapShape();
				}
				if (minimap_drag) {
					auto p = sf::Vector2f(sf::Mouse::getPosition(window)) - sf::Vector2f(minimap_border_shape.getPosition());
					float mx = p.x/200.f * WORLD_W;
					float my = p.y/200.f * WORLD_H;
					mx = min(mx, float(WORLD_W + WINDOW_W/2.f));
					mx = max(mx, float(-WINDOW_W/2.f));
					my = min(my, float(WORLD_H + WINDOW_H/2.f));
					my = max(my, float(-WINDOW_H/2.f));

					game_view.setCenter({mx, my});
					UpdateGameViewMinimapShape();
				}
			}

			if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::LControl) {
					gui_info_zoom->active = true;
				}
			}
			else if (event.type == sf::Event::KeyReleased) {
				if (event.key.code == sf::Keyboard::LControl) {
					gui_info_zoom->active = false;
				}
			}

			if (console->getActive()) { if (console->HandleEvent(event)) continue; }

		}

		if (escape_pressed && quit_timer.getElapsedTime() >= QUICK_EXIT_TIME) {
			Quit();
		}

		{
			int i = 0;
			for (auto gi : gui_infos) {
				if (gi->active) {
					gi->text_obj.setPosition(10.f, float(10 + 15*i));
					++i;
					if (gi->name == "mouse (world)") {
						gi->setVal("{" + to_string(int(mpos.x)) + ", " + to_string(int(mpos.y)) + "}");
					}
				}
			}
		}

		if (world_init) world.Update(dt);
		if (console->getActive()) { console->Update(); }

		window.clear(sf::Color::White);

		window.setView(game_view);
		world.Render(window);


		window.setView(window.getDefaultView());
		for (auto gi : gui_infos) {
			if (gi->active) {
				window.draw(gi->text_obj);
			}
		}
		if (selected_entity) window.draw(selected_entity_pos_label->text_obj);
		if (console->getActive()) { console->Render(window); }
		window.draw(minimap_border_shape);

		minimap_texture.clear();
		world.Render(minimap_texture);
		minimap_texture.display();
		window.draw(minimap_sprite);

		window.setView(window.getDefaultView());
		window.draw(game_view_minimap_shape);

		window.display();

		dt = dt_clock.restart().asMicroseconds() / 1000.f;
		++frames;
	}

}

void Editor::Quit()
{
	window.close();
}

void Editor::UpdateGameViewMinimapShape()
{
	float sx = game_view.getCenter().x / WORLD_W * 200.f;
	float sy = game_view.getCenter().y / WORLD_H * 200.f;
	float sw = game_view.getSize().x / WORLD_W * 200.f;
	float sh = game_view.getSize().y / WORLD_H * 200.f;

	game_view_minimap_shape.setSize({sw, sh});
	game_view_minimap_shape.setPosition(WINDOW_W-204 + sx - sw/2.f, 4+sy-sh/2.f);
}

void Editor::ToggleGroundEditMode()
{
	ground_edit_mode = !ground_edit_mode;
	ground_edit_info->active = ground_edit_mode;
	ground_edit_info->setVal(getGroundTypeString(GroundType(ground_type)));

	if (ground_edit_mode) {
		
	}
	else {

	}
}
