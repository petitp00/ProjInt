#pragma once

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/View.hpp>

#include <functional>

#include "Game.h"
#include "Tweener.h"

static sf::Color BG_COLOR = sf::Color(139, 146, 158);
static sf::Color BG_HOVER = sf::Color(41, 48, 61);


// Forward declaration
class Tooltip;
class MenuPage;

class ButtonActionImpl {
public:
	ButtonActionImpl(Game& game, MenuState& menu_state, GameState& game_state);

	Game& game;
	MenuState& menu_state;
	GameState& game_state;

	// variable refs
	bool* mute_active_ref;
	float* volume_slider_ref;
	std::vector<sf::Keyboard::Key*> controls_values;
	std::string* world_name_ref;
};

using action_t = std::function<void(ButtonActionImpl*)>*;

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
	virtual void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
						bool draw_on_tooltip_render_target=false);

	virtual bool onClick(sf::Vector2i mouse_pos) { clicked = true; return true; }
	virtual bool onClickRelease() { clicked = false; return true; }
	virtual bool onHoverIn(sf::Vector2i mouse_pos={0,0});
	virtual bool onHoverOut();
	virtual bool onMouseWheel(float delta) { return false; }
	//virtual bool onKeyPressed(sf::Keyboard::Key key) { return false; }
	virtual bool onKeyType(sf::Event::KeyEvent e) { return false; }

	// when mouse has moved on top of object
	virtual void UpdateHoveredMousePos(sf::Vector2i mouse_pos);

	virtual void UpdateClickDrag(sf::Vector2i mouse_pos) {}

	bool isMouseIn(sf::Vector2i mouse_pos);
	bool isClicked() { return clicked; }

	virtual void setPos(sf::Vector2f pos) { this->pos = pos; }
	virtual void setSize(sf::Vector2f size) { this->size = size; }
	virtual void setTooltip(Tooltip* tooltip) { this->tooltip = tooltip; }
	virtual void setOnClickAction(action_t action, ButtonActionImpl* impl) {
		this->button_action_impl = impl; this->action = action;
	}
	virtual void setActive(bool active) { this->active = active; }

	sf::Vector2f getPos() { return pos; }
	sf::Vector2f getSize() { return size; }
	bool getHovered() { return hovered; }
	bool getActive() { return active; }

protected:
	sf::Vector2f pos;
	sf::Vector2f size;

	bool clicked = false;
	bool hovered = false;
	bool active = false; // only one item can be active at once

	Tooltip* tooltip = nullptr;
	ButtonActionImpl* button_action_impl = nullptr;
	action_t action = nullptr;

	// ADD ID? (maybe with static int)
};

/*
	A text container with word wrapping
*/
class TextBox : public GUIObject
{
public:
	TextBox() = default;
	TextBox(std::string const& text_string, sf::Vector2f pos, float width, sf::Font* font,
			sf::Color color, unsigned int character_size);

	TextBox(std::string const& text_string, sf::Vector2f pos, float width,
			std::string const& font_name, sf::Color color, unsigned int character_size);

	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
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

// TEXT BUTTON CLASS
class TextButton : public GUIObject {
public:
	TextButton()=default;
	TextButton(std::string const& text_string, sf::Vector2f pos, float width,
			   unsigned int character_size = FontSize::NORMAL,
			   sf::Color text_color = sf::Color::Black,
			   sf::Color background_color = BG_COLOR,
			   sf::Color background_color_hover = BG_HOVER,
			   std::string const& font_name=BASE_FONT_NAME);


	void Update() override;
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(sf::Vector2i mouse_pos) override;
	bool onHoverIn(sf::Vector2i mouse_pos={0,0}) override;
	bool onHoverOut() override;

	void setPos(sf::Vector2f pos) override;

protected:
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

/*
Button for controls selection
*/
class ControlsTextButton : public TextButton
{
public:
	ControlsTextButton()=default;
	ControlsTextButton(std::string const& text_string, sf::Vector2f pos, float width,
					   unsigned int character_size = FontSize::NORMAL,
					   sf::Color text_color = sf::Color::Black,
					   sf::Color background_color = BG_COLOR,
					   sf::Color background_color_hover = BG_HOVER,
					   std::string const& font_name=BASE_FONT_NAME);

	bool onClick(sf::Vector2i mouse_pos) override;
	bool onHoverIn(sf::Vector2i mouse_pos={0,0}) override;
	bool onHoverOut() override;
	//bool onKeyPressed(sf::Keyboard::Key key) override;
	bool onKeyType(sf::Event::KeyEvent e) override;

	void setActive(bool active) override;
	void setKey(sf::Keyboard::Key key) { this->key = key; }
	sf::Keyboard::Key* getKeyRef() { return &key; }

private:
	bool UpdateControlsTextButton(bool set_params = true);

	sf::Keyboard::Key key;
};

class Tooltip : public GUIObject {
public:
	Tooltip()=default;
	Tooltip(std::string const& text_string, sf::Time show_after);


	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
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
	Checkbox(bool active, sf::Vector2f pos, sf::Vector2f size ={40, 40},
			 sf::Color background_color = BG_COLOR, sf::Color background_color_hover = BG_HOVER );

	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(sf::Vector2i mouse_pos) override;
	bool onHoverIn(sf::Vector2i mouse_pos={0,0}) override;
	bool onHoverOut() override;

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
	Slider(sf::Vector2f pos, float width, float start_value, float min_value, float max_value,
		   sf::Color background_color = BG_COLOR, sf::Color background_color_hover = BG_HOVER );

	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(sf::Vector2i mouse_pos) override;
	bool onHoverIn(sf::Vector2i mouse_pos={0,0}) override;
	bool onHoverOut() override;
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

class Scrollbar : public GUIObject {
public:
	Scrollbar()=default;
	Scrollbar(sf::Vector2f pos, float height, float start_val, float min_value, float max_value,
			  sf::Color background_color = BG_COLOR, sf::Color background_color_hover = BG_HOVER
	);

	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(sf::Vector2i mouse_pos) override;
	bool onHoverIn(sf::Vector2i mouse_pos={0,0}) override;
	bool onHoverOut() override;
	void UpdateClickDrag(sf::Vector2i mouse_pos);

	float getValue() { return value; }
	float* getValueRef() { return &value; }

	void setMaxValue(float max_value) { this->max_value = max_value; }

	void UpdateScrollbar(float val);
private:

	sf::RectangleShape bar_shape;
	sf::RectangleShape rect_shape;

	sf::Vector2f bar_pos;
	float bar_height;

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
	ObjContainer(sf::Vector2f pos, sf::Vector2f size);

	~ObjContainer() { delete scrollbar; }

	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	void AddObject(GUIObject* obj);

	bool onClick(sf::Vector2i mouse_pos) override;
	bool onClickRelease() override;
	bool onHoverIn(sf::Vector2i mouse_pos={0,0}) override;
	bool onHoverOut() override;
	bool onMouseWheel(float delta) override;
	//bool onKeyPressed(sf::Keyboard::Key key) override;
	bool onKeyType(sf::Event::KeyEvent e) override;

	void UpdateClickDrag(sf::Vector2i mouse_pos) override;
	void UpdateHoveredMousePos(sf::Vector2i mouse_pos) override;

private:
	void UpdateObjContainer(bool set_params = true);

	std::vector<GUIObject*> gui_objects;
	Scrollbar* scrollbar;

	float y_offset = 0.f;
	float min_offset = 0.0f;
	float max_offset = 1.f;
	sf::View view;
	sf::RenderTexture render_texture;
	sf::Sprite render_sprite;

	sf::RectangleShape rect_shape;
};

class TextInputBox : public GUIObject {
public:
	TextInputBox()=default;
	TextInputBox(sf::Vector2f pos, float width, unsigned int character_size = FontSize::SMALL);

	void Update() override;
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(sf::Vector2i mouse_pos) override;
	void UpdateClickDrag(sf::Vector2i mouse_pos);
	bool onKeyType(sf::Event::KeyEvent e) override;
	std::string* getStringRef() { return &text_string; }

private:
	void Init(); // sets the size (height) and the positions of stuff
	void UpdateText(); // updates the text's string and such

	std::string text_string;
	unsigned int cursor_pos = 0;

	float width = 0;
	int character_size;

	sf::RectangleShape rect_shape;
	sf::RectangleShape cursor_shape;
	sf::Text text_obj;

	sf::Clock clock;
	sf::Time cursor_blink_time = sf::seconds(0.5f);
	bool show_cursor = true;
};
