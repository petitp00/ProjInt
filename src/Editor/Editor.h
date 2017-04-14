#pragma once

#include <SFML/Graphics.hpp>

#include "../World.h"
#include "../Game.h"
#include "../ResourceManager.h"
#include "../GameGUI.h"

namespace ConsoleNamespace { class Console; };


namespace EditorMode {
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
		void Quit() { window.close(); }

		void UpdateAfterWindowResize();
		void UpdateGameViewMinimapShape();
		void ToggleGroundEditMode();
		
		ConsoleNamespace::Console* console;
		Controls* controls;

		World world;
		Inventory inventory;
		bool world_init = false;

	private:
		sf::Clock quit_timer;
		sf::RenderWindow window;
		std::vector<GUIInfo*> gui_infos;

		sf::View window_view;

		// Game view
		sf::View game_view;
		float game_view_zoom = 1.0f;

		// Minimap view
		sf::View minimap_view;
		sf::RenderTexture minimap_texture;
		sf::Sprite minimap_sprite;
		sf::RectangleShape minimap_border_shape;
		sf::RectangleShape game_view_minimap_shape;

		// Ground Edit mode
		bool ground_edit_mode = false;
		GUIInfo* ground_edit_info = nullptr;
		int ground_type = NONE;
		sf::VertexArray grid;
	};
}