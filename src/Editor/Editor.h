#pragma once

#include <SFML/Graphics.hpp>

#include "../World.h"
#include "../Game.h"
#include "../ResourceManager.h"

namespace ConsoleNamespace { class Console; };


namespace EditorMode {
	static const int WINDOW_W = 1280;
	static const int WINDOW_H = 720;

	static const int WORLD_W = 1280*3;
	static const int WORLD_H = 1280*3;

	struct GUIInfo {
		void setVal(const std::string& val) {
			text_obj.setString(name + ": " + val);
		}
		std::string name;
		sf::Text text_obj;
		bool active = true;
	};

	static GUIInfo* make_info(const std::string& name, bool active = true) {
		auto gi = new GUIInfo;
		gi->name = name;
		gi->active = active;
		gi->text_obj.setFont(ResourceManager::getFont(BASE_FONT_NAME));
		gi->text_obj.setCharacterSize(15);
		gi->text_obj.setFillColor(sf::Color::Black);
		return gi;
	}

	class Editor
	{
	public:
		Editor();
		~Editor();

		void Start();
		void Quit();

		sf::RenderWindow window;
		ConsoleNamespace::Console* console;
		Controls* controls;

		World world;
		bool world_init = false;

		void UpdateGameViewMinimapShape();

		void ToggleGroundEditMode();
		

	private:
		sf::Clock quit_timer;

		std::vector<GUIInfo*> gui_infos;

		sf::View game_view;
		sf::View minimap_view;
		sf::Vector2f game_view_size;
		sf::Vector2f minimap_view_size;
		float game_view_zoom = 1.0f;
		sf::RectangleShape game_view_minimap_shape;

		bool ground_edit_mode = false;
		GUIInfo* ground_edit_info = nullptr;
	};
}