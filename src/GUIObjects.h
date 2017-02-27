#pragma once

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/View.hpp>

#include <functional>

#include "Game.h"
#include "Tweener.h"

// Forward declaration
class Tooltip;

class ButtonActionImpl {
public:
	ButtonActionImpl(Game& game);

	Game& game;
	bool* mute_active_ref;
	float* volume_slider_ref;
};


/*
	Base GUI Object class
*/
class GUIObject
{
public:
	GUIObject()=default;
	GUIObject(sf::Vector2f pos, sf::Vector2f size);
	virtual ~GUIObject() = 0;

	virtual void Update();
	virtual void Render(sf::RenderTarget& target,
						sf::RenderTarget& tooltip_render_target,
						bool draw_on_tooltip_render_target=false);

	virtual void onClick() { clicked = true; }
	virtual void onClickRelease() { clicked = false; }
	virtual void onHoverIn(sf::Vector2i mouse_pos={ 0,0 });
	virtual void onHoverOut();
	virtual void onMouseWheel(float delta) {}

	// when mouse has moved on top of object
	virtual void UpdateHoveredMousePos(sf::Vector2i mouse_pos);

	virtual void UpdateClickDrag(sf::Vector2i mouse_pos) {}

	bool isMouseIn(sf::Vector2i mouse_pos);
	bool isClicked() { return clicked; }

	virtual void setPos(sf::Vector2f pos) { this->pos = pos; }
	virtual void setSize(sf::Vector2f size) { this->size = size; }
	virtual void setTooltip(Tooltip* tooltip) { this->tooltip = tooltip; }
	virtual void setOnClickAction(
		std::function<void(ButtonActionImpl*)>* action,
		ButtonActionImpl* impl) {
		this->button_action_impl = impl; this->action = action;
	}

	sf::Vector2f getPos() { return pos; }
	sf::Vector2f getSize() { return size; }
	bool getHovered() { return hovered; }

protected:
	sf::Vector2f pos;
	sf::Vector2f size;

	bool clicked = false;
	bool hovered = false;

	Tooltip* tooltip = nullptr;
	ButtonActionImpl* button_action_impl = nullptr;
	std::function<void(ButtonActionImpl*)>* action = nullptr;

	// ADD ID? (maybe with static int)
};

/*
	A text container with word wrapping
*/
class TextBox : public GUIObject
{
public:
	TextBox() = default;
	TextBox(std::string const& text_string,
			sf::Vector2f pos, float width,
			sf::Font* font, sf::Color color,
			unsigned int character_size);

	TextBox(std::string const& text_string,
			sf::Vector2f pos, float width,
			std::string const& font_name, sf::Color color,
			unsigned int character_size);

	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;


	void setPos(sf::Vector2f pos) override { setPos(pos, true); }
	void setPos(sf::Vector2f pos, bool update);
	void setWidth(float width, bool update=true);
	void setFont(sf::Font* font, bool update=true);
	void setFont(std::string const& font_name, bool update=true);
	void setColor(sf::Color color, bool update=true);
	void setCharacterSize(unsigned int character_size, bool update=true);
	void setTextString(std::string const& text_string, bool update=true);

	sf::Font* getFont() { return font; }
	sf::Color getColor() { return color; }
	unsigned int getCharacterSize() { return character_size; }
	std::string getTextString() { return text_string; }

private:
	void UpdateTextBox(bool set_text_obj_params = true, int start_at_index = 0);

	sf::Font* font;
	sf::Color color;
	unsigned int character_size;

	std::string text_string;
	std::string original_text_string;

	sf::Text text_obj;
};

class TextButton : public GUIObject {
public:
	TextButton()=default;
	TextButton(std::string const& text_string,
			   sf::Vector2f pos, float width,
			   unsigned int character_size = FontSize::NORMAL,
			   sf::Color text_color = sf::Color::Black,
			   sf::Color background_color = sf::Color(139, 146, 158),
			   sf::Color background_color_hover = sf::Color(41, 48, 61),
			   std::string const& font_name=BASE_FONT_NAME);


	void Update() override;
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	void onClick() override;
	void onHoverIn(sf::Vector2i mouse_pos={ 0,0 }) override;
	void onHoverOut() override;

	void setPos(sf::Vector2f pos) override;

private:
	void UpdateTextButton(bool set_params = true);


	sf::Font* font;
	unsigned int character_size;
	float width = 0; // 0 for automatic
	float margin = 16;

	sf::Color text_color;
	sf::Color background_color;
	sf::Color background_color_hover;

	Tweener color_tw;

	std::string text_string;
	sf::Text text_obj;
	sf::RectangleShape rect_shape;
};

class Tooltip : public GUIObject {
public:
	Tooltip()=default;
	Tooltip(std::string const& text_string, sf::Time show_after);


	void Update() override;
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=true) override;

	void StartTimer(sf::Vector2i mouse_pos);
	void StopTimer();

	void setMousePos(sf::Vector2i mouse_pos);

private:
	TextBox text_box;
	sf::RectangleShape rect_shape;

	bool timer_active = false;
	sf::Clock clock;
	sf::Time show_after;

	bool alpha_tweener_started = false;
	Tweener alpha_tweener;
};

class Checkbox : public GUIObject {
public:
	Checkbox() = default;
	Checkbox(bool active, sf::Vector2f pos,
			 sf::Vector2f size ={ 40, 40 },
			 sf::Color background_color = sf::Color(139, 146, 158),
			 sf::Color background_color_hover = sf::Color(41, 48, 61)
	);

	void Update() override;
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	void onClick() override;
	void onHoverIn(sf::Vector2i mouse_pos={ 0,0 }) override;
	void onHoverOut() override;

	bool* getActiveRef() { return &active; }

private:
	void UpdateCheckbox();

	sf::Text text_obj;
	sf::RectangleShape rect_shape;

	bool active = false;

	sf::Color background_color;
	sf::Color background_color_hover;

	Tweener color_tw;
};

class Slider : public GUIObject {
public:
	Slider()=default;
	Slider(sf::Vector2f pos,
		   float width,
		   float start_value,
		   float min_value,
		   float max_value,
		   sf::Color background_color = sf::Color(139, 146, 158),
		   sf::Color background_color_hover = sf::Color(41, 48, 61)
	);

	void Update() override;
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	void onClick() override;
	void onHoverIn(sf::Vector2i mouse_pos={ 0,0 }) override;
	void onHoverOut() override;
	void UpdateClickDrag(sf::Vector2i mouse_pos);

	float* getValueRef() { return &value; }

private:
	void UpdateSlider();

	sf::RectangleShape bar_shape;
	sf::RectangleShape rect_shape;

	sf::Vector2f bar_pos;
	float bar_width;

	float value;
	float min_value;
	float max_value;

	sf::Color background_color;
	sf::Color background_color_hover;

	Tweener color_tw;
};

class ObjContainer : public GUIObject {
public:
	ObjContainer()=default;
	ObjContainer(sf::Vector2f pos,
				 sf::Vector2f size);

	void Update() override;
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	void AddObject(GUIObject* obj);

	void onClick() override;
	void onClickRelease() override;
	void onHoverIn(sf::Vector2i mouse_pos={ 0,0 }) override;
	void onHoverOut() override;
	void onMouseWheel(float delta) override;
	void UpdateClickDrag(sf::Vector2i mouse_pos) override;
	void UpdateHoveredMousePos(sf::Vector2i mouse_pos) override;

private:
	void UpdateObjContainer(bool set_params = true);

	std::vector<GUIObject*> gui_objects;

	float y_offset = 0.f;
	sf::View view;
	sf::RenderTexture render_texture;
	sf::Sprite render_sprite;

	sf::RectangleShape rect_shape;
}; 
