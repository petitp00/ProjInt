#include "Editor.h"
#include "../Console.h"
#include "../ResourceManager.h"

#include <iostream>

using namespace EditorMode;
using namespace std;

// Vars
static vec2 game_view_size ={float(WINDOW_WIDTH), float(WINDOW_HEIGHT)};

static bool escape_pressed	= false;
static bool middle_pressed	= false;
static bool left_pressed	= false;
static bool minimap_drag	= false;

static vec2i mouse_pos;
static vec2 mouse_pos_in_world;
static vec2 drag_mouse_pos;
static vec2 last_ground_edit {-1.f, -1.f};

static Entity* selected_entity = nullptr;
static vec2 selected_entity_click_offset;

// Forward decl
void InitMinimap(sf::RenderTexture& texture,
				 sf::Sprite& sprite,
				 sf::RectangleShape& border_shape,
				 sf::RectangleShape& game_view_shape);

Editor::Editor() :
	game_view(sf::FloatRect({0,0}, game_view_size)),
	minimap_view(sf::FloatRect({0,0}, {float(WORLD_W), float(WORLD_H)})),
	controls(new Controls),
	inventory(controls)
{
	window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Editor", sf::Style::Default);
	
	//Item::Init();

	controls->LoadDefault(); 
	world.controls = controls;
	world.Init(&inventory, nullptr, nullptr);

	console = new ConsoleNamespace::Console(*this);

	window_view = window.getDefaultView();


	//grid
	sf::Color grid_color(sf::Color::Magenta);
	grid.setPrimitiveType(sf::Lines);
	for (int x = 0; x < ((WORLD_W+1)/visual_tile_size); ++x) {
		grid.append(sf::Vertex({float(x*visual_tile_size), 0}, grid_color));
		grid.append(sf::Vertex({float(x*visual_tile_size), float(WORLD_H)}, grid_color));
	}
	for (int y = 0; y < ((WORLD_H+1)/visual_tile_size); ++y) {
		grid.append(sf::Vertex({0, float(y*visual_tile_size)}, grid_color));
		grid.append(sf::Vertex({float(WORLD_W), float(y*visual_tile_size)}, grid_color));
	}
}

Editor::~Editor()
{
	delete console;
}

void Editor::Start()
{
	// Framerate stuff
	sf::Clock clock;
	sf::Clock refresh_clock;
	sf::Time refresh(sf::seconds(0.25f));
	int frames = 0;
	sf::Clock dt_clock;
	float dt = 0;

	// Gui infos
	ground_edit_info				= make_info("ground_edit", false);
	auto fps_info					= make_info("fps");
	auto gui_info_id				= make_info("id", false);
	auto gui_info_zoom				= make_info("zoom", false);
	auto selected_entity_pos_label	= make_info("");
	gui_infos.push_back(ground_edit_info);
	gui_infos.push_back(fps_info);
	gui_infos.push_back(make_info("mouse (world)"));
	gui_infos.push_back(gui_info_id);
	gui_infos.push_back(gui_info_zoom);
	
	// Minimap
	InitMinimap(minimap_texture, minimap_sprite, minimap_border_shape, game_view_minimap_shape);
	minimap_texture.setView(minimap_view);

	
	while (window.isOpen()) {

		// FPS counter
		if (refresh_clock.getElapsedTime() >= refresh) {
			refresh_clock.restart();
			float t = clock.restart().asMicroseconds() / 1000.f;
			fps_info->setVal(to_string(int(1000 / (t / frames))));

			frames = 0;
		}

		//		  //
		// Events //
		//		  //

		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) { Quit(); }

			// KEYBOARD EVENTS
			if (event.type == sf::Event::KeyPressed) { 
				auto key = event.key.code;
				if (key == sf::Keyboard::Escape) {
					if (!escape_pressed) {
						quit_timer.restart();
						escape_pressed = true;
					}
				}
				else if (key == sf::Keyboard::LControl) {
					gui_info_zoom->active = true;
				}
				else if (key == sf::Keyboard::F1) {
					console->setActive(true);
				}
				if (!console->getActive()) {
					if (key == sf::Keyboard::F) {
						if (ground_edit_mode) {
							world.ground.Fill(mouse_pos_in_world, GroundType(ground_type));
						}
					}
					else if (key == sf::Keyboard::D) {
						auto e = world.FindEntityClicked(mouse_pos_in_world);
						if (e) {
							world.DuplicateEntity(e->getId());
						}
					}
					else if (key == sf::Keyboard::Delete) {
						auto e = world.FindEntityClicked(mouse_pos_in_world);
						if (e) {
							world.DeleteEntity(e->getId());
						}
					}
				}
			} 

			else if (event.type == sf::Event::KeyReleased) {
				auto key = event.key.code;
				if (key == sf::Keyboard::Escape) {
					escape_pressed = false;
				}
				else if (key == sf::Keyboard::LControl) {
					gui_info_zoom->active = false;
				}
			}

			// MOUSE EVENTS
			else if (event.type == sf::Event::MouseButtonPressed) {
				auto button = event.mouseButton.button;
				if (button == sf::Mouse::Left) {
					left_pressed = true;

					// Check for minimap view drag
					sf::FloatRect rect(minimap_border_shape.getPosition(), minimap_border_shape.getSize());
					if (rect.contains(vec2(mouse_pos))) {
						minimap_drag = true;
						drag_mouse_pos = vec2(mouse_pos) - minimap_border_shape.getPosition();
					}
					// Change tile if ground edit mode
					else if (ground_edit_mode) {
						world.ground.setTileClicked(mouse_pos_in_world, GroundType(ground_type));
					}
					// Check if entity clicked
					else {
						auto e = world.FindEntityClicked(mouse_pos_in_world);
						if (e) {
							selected_entity_click_offset = mouse_pos_in_world - e->getPos();
							selected_entity = e;
							gui_info_id->active = true;
							gui_info_id->setVal(to_string(selected_entity->getId()));
							selected_entity_pos_label->text_obj.setPosition(
								vec2(window.mapCoordsToPixel(selected_entity->getPos() + vec2(0, selected_entity->getSize().y + 10.f), game_view)));
						}
					}
				}
				else if (button == sf::Mouse::Right) {
					// Copy tile type if ground edit mode
					if (ground_edit_mode) {
						ground_type = int(world.ground.getTileClicked(mouse_pos_in_world));
						ground_edit_info->setVal(getGroundTypeString(GroundType(ground_type)));
					}
				}
				else if (button == sf::Mouse::Middle) {
					auto e = world.FindEntityClicked(mouse_pos_in_world);
					if (e) {
						console->PrintInfo("Entity clicked");
						console->PrintInfo("ID: " + to_string(e->getId()));
						console->PrintInfo("Type: " + to_string(e->getType()) + "   (" + getEntityTypeString(e->getType()) + ")");
						console->PrintInfo("Pos: {" + to_string(e->getPos().x) + ", " + to_string(e->getPos().y) + "}");
					}
					
					middle_pressed = true;
					drag_mouse_pos = mouse_pos_in_world;
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased) {
				auto button = event.mouseButton.button;
				if (button == sf::Mouse::Left) {
					minimap_drag = false;
					left_pressed = false;
					if (selected_entity) selected_entity = nullptr;
				}
				else if (button == sf::Mouse::Middle) {
					middle_pressed = false;
				}
			}

			if (event.type == sf::Event::MouseWheelScrolled) {
				auto delta = event.mouseWheelScroll.delta;
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
					// Zoom
					float factor = 0.15f;
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) factor = 0.45f;
					if		(delta < 0) game_view_zoom += factor;
					else if (delta > 0) game_view_zoom -= factor;

					if (game_view_zoom < 0.001f) game_view_zoom = 0.001f;

					gui_info_zoom->setVal(to_string(game_view_zoom));
					game_view.setSize(game_view_size * game_view_zoom);
					UpdateGameViewMinimapShape();
				}
				else if (ground_edit_mode) {
					// Change tile type
					if (delta > 0) {
						if (ground_type != ground_type_max) ++ground_type;
						else								ground_type = 0;
					}
					else if (delta < 0) {
						if (ground_type != 0)				--ground_type;
						else								ground_type = ground_type_max;
					}
					ground_edit_info->setVal(getGroundTypeString(GroundType(ground_type)));
				}
			}

			if (event.type == sf::Event::MouseMoved) {
				mouse_pos = sf::Mouse::getPosition(window);
				mouse_pos_in_world = window.mapPixelToCoords(sf::Mouse::getPosition(window), game_view);

				if (selected_entity) {
					// Move selected entity
					selected_entity->setPos(mouse_pos_in_world - selected_entity_click_offset);
					selected_entity_pos_label->setVal("{"+ to_string(int(selected_entity->getPos().x)) + ", " + to_string(int(selected_entity->getPos().y)) + "}");
					selected_entity_pos_label->text_obj.setPosition( vec2( window.mapCoordsToPixel(
						selected_entity->getPos() + vec2(0, selected_entity->getSize().y + 10.f), game_view)));

				}
				else if (left_pressed && ground_edit_mode) {
					// Add tiles to ground
					vec2 mouse_pos_in_tile_pos {
						float(int(mouse_pos_in_world.x/Ground::getVisualTileSize())),
						float(int(mouse_pos_in_world.y/Ground::getVisualTileSize()))
					};

					if (mouse_pos_in_tile_pos != last_ground_edit) {
						last_ground_edit = mouse_pos_in_tile_pos;
						world.ground.setTileClicked(mouse_pos_in_world, GroundType(ground_type));
					}
				}
				else {
					// Find if hover on entity
					auto e = world.FindEntityClicked(mouse_pos_in_world);
					if (e) {
						gui_info_id->active = true;
						gui_info_id->setVal(to_string(e->getId()));
					}
					else {
						gui_info_id->active = false;
					}
				}

				if (middle_pressed) {
					game_view.setCenter(game_view.getCenter() + drag_mouse_pos - mouse_pos_in_world);
					UpdateGameViewMinimapShape();
				}
				if (minimap_drag) {
					auto p = vec2(sf::Mouse::getPosition(window)) - vec2(minimap_border_shape.getPosition());
					float mx = p.x/200.f * WORLD_W;
					float my = p.y/200.f * WORLD_H;
					mx = min(mx, float(WORLD_W + WINDOW_WIDTH/2.f));
					mx = max(mx, float(-WINDOW_WIDTH/2.f));
					my = min(my, float(WORLD_H + WINDOW_HEIGHT/2.f));
					my = max(my, float(-WINDOW_HEIGHT/2.f));

					game_view.setCenter({mx, my});
					UpdateGameViewMinimapShape();
				}
			}
			else if (event.type == sf::Event::Resized) {
				WINDOW_WIDTH = event.size.width;
				WINDOW_HEIGHT = event.size.height;
				UpdateAfterWindowResize();
			}


			if (console->getActive()) { if (console->HandleEvent(event)) continue; }

		}

		if (escape_pressed && quit_timer.getElapsedTime() >= QUICK_EXIT_TIME) {
			Quit();
		}

		//		   //
		// UPDATES //
		//		   //

		// Setting active gui_infos' position
		int i = 0;
		for (auto gi : gui_infos) {
			if (gi->active) {
				gi->text_obj.setPosition(10.f, float(10 + 15*i));
				++i;
				if (gi->name == "mouse (world)") {
					gi->setVal("{" + to_string(int(mouse_pos_in_world.x)) + ", " + to_string(int(mouse_pos_in_world.y)) + "}");
				}
			}
		}

		if (world_init)					world.Update(dt, mouse_pos_in_world);
		if (console->getActive())		console->Update();

		//			 //
		// RENDERING //
		//			 //

		window.clear(sf::Color::White);

		window.setView(game_view);
		{
			world.Render(window);

			if (ground_edit_mode) {
				window.draw(grid);
			}
		}

		window.setView(window_view);
		{
			// gui infos
			for (auto gi : gui_infos) {
				if (gi->active)			window.draw(gi->text_obj);
			}
			if (selected_entity)		window.draw(selected_entity_pos_label->text_obj);

			// minimap
			window.draw(minimap_border_shape);
			minimap_texture.clear();
			world.Render(minimap_texture);
			minimap_texture.display();
			window.draw(minimap_sprite);
			window.draw(game_view_minimap_shape);

			// console
			if (console->getActive())	console->Render(window);
		}

		window.display();

		// FPS counter
		dt = dt_clock.restart().asMicroseconds() / 1000.f;
		++frames;
	}

}

void Editor::UpdateAfterWindowResize()
{
	game_view_size ={float(WINDOW_WIDTH), float(WINDOW_HEIGHT)};
	game_view.reset(sf::FloatRect({0,0}, game_view_size));
	game_view.setSize(game_view_size * game_view_zoom);

	window_view.reset(sf::FloatRect(0, 0, float(WINDOW_WIDTH), float(WINDOW_HEIGHT)));

	minimap_view.reset(sf::FloatRect({0,0}, {float(WORLD_W), float(WORLD_H)}));
	InitMinimap(minimap_texture, minimap_sprite, minimap_border_shape, game_view_minimap_shape);
	minimap_texture.setView(minimap_view);

	console->UpdateResize();
}

void Editor::UpdateGameViewMinimapShape()
{
	float sx = game_view.getCenter().x / WORLD_W * 200.f;
	float sy = game_view.getCenter().y / WORLD_H * 200.f;
	float sw = game_view.getSize().x / WORLD_W * 200.f;
	float sh = game_view.getSize().y / WORLD_H * 200.f;

	game_view_minimap_shape.setSize({sw, sh});
	game_view_minimap_shape.setPosition(WINDOW_WIDTH-204 + sx - sw/2.f, 4+sy-sh/2.f);
}

void Editor::ToggleGroundEditMode()
{
	ground_edit_mode = !ground_edit_mode;
	ground_edit_info->active = ground_edit_mode;
	ground_edit_info->setVal(getGroundTypeString(GroundType(ground_type)));
}

// Functions

void InitMinimap(sf::RenderTexture& texture,
				 sf::Sprite& sprite,
				 sf::RectangleShape& border_shape,
				 sf::RectangleShape& game_view_shape) {
	texture.create(200, 200);
	sprite.setTexture(texture.getTexture());
	sprite.setColor(sf::Color(255, 255, 255, 128));
	sprite.setPosition(float(WINDOW_WIDTH-204), 4);
	border_shape.setSize(vec2(200, 200));
	border_shape.setFillColor(sf::Color::Transparent);
	border_shape.setOutlineColor(sf::Color::Black);
	border_shape.setOutlineThickness(3);
	border_shape.setPosition(float(WINDOW_WIDTH - 204), 4);
	game_view_shape.setFillColor(sf::Color::Transparent);
	game_view_shape.setOutlineColor(sf::Color::Green);
	game_view_shape.setOutlineThickness(2);
}
