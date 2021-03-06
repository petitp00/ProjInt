#pragma once

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/View.hpp>

#include <functional>

#include "Game.h"
#include "Tweener.h"
#include "Items.h"
#include "GameGUI.h"

static sf::Color BG_COLOR = sf::Color(139, 146, 158);
static sf::Color BG_HOVER = sf::Color(41, 48, 61);


// Forward declaration
class Tooltip;
class MenuPage;

class ButtonActionImpl {
public:
	ButtonActionImpl(Game* game, MenuState* menu_state, GameState* game_state);

	Game* game;
	MenuState* menu_state;
	GameState* game_state;

	// variable refs
	bool* mute_active_ref;
	float* volume_slider_ref;
	std::vector<sf::Keyboard::Key*> controls_values;
	std::string* world_name_ref;
	std::string load_world_name;
	Item::Recipe recipe;
	int item;
};

using action_t = std::function<void(ButtonActionImpl*)>*;

/*
	Base GUI Object class
*/
class GUIObject
{
public:
	GUIObject()=default;
	GUIObject(vec2 pos, vec2 size);
	virtual ~GUIObject() = 0;

	virtual void Update();
	virtual void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
						bool draw_on_tooltip_render_target=false);

	virtual bool onClick(vec2i mouse_pos) { clicked = true; return true; }
	virtual bool onClickRelease() { clicked = false; return true; }
	virtual bool onHoverIn(vec2i mouse_pos={0,0});
	virtual bool onHoverOut();
	virtual bool onMouseWheel(float delta) { return false; }
	//virtual bool onKeyPressed(sf::Keyboard::Key key) { return false; }
	virtual bool onKeyType(sf::Event::KeyEvent e) { return false; }

	// when mouse has moved on top of object
	virtual void UpdateHoveredMousePos(vec2i mouse_pos);

	virtual void UpdateClickDrag(vec2i mouse_pos) {}

	bool isMouseIn(vec2i mouse_pos);
	bool isClicked() { return clicked; }

	virtual void setPos(vec2 pos) { this->pos = pos; }
	virtual void setOrigin(vec2 origin) { this->origin = origin; }
	virtual void setSize(vec2 size) { this->size = size; }
	virtual void setTooltip(Tooltip* tooltip) { this->tooltip = tooltip; }
	virtual void setOnClickAction(action_t action, ButtonActionImpl* impl) {
		this->button_action_impl = impl; this->action = action;
	}
	virtual void setActive(bool active) { this->active = active; }

	vec2 getPos() { return pos; }
	vec2 getOrigin() { return origin; }
	vec2 getSize() { return size; }
	bool getHovered() { return hovered; }
	bool getActive() { return active; }

protected:
	vec2 pos;
	vec2 origin;
	vec2 size;

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
	TextBox(std::string const& text_string, vec2 pos, float width, sf::Font* font,
			sf::Color color, unsigned int character_size);

	TextBox(std::string const& text_string, vec2 pos, float width,
			std::string const& font_name, sf::Color color, unsigned int character_size);

	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;


	void setPos(vec2 pos) override { setPos(pos, true); }
	void setPos(vec2 pos, bool update);
	void setOrigin(vec2 origin) override;
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
	float width;

	std::string text_string;
	std::string original_text_string;

	sf::Text text_obj;
};

// TEXT BUTTON CLASS
class TextButton : public GUIObject {
public:
	TextButton()=default;
	TextButton(std::string const& text_string, vec2 pos, float width,
			   unsigned int character_size = FontSize::NORMAL,
			   sf::Color text_color = sf::Color::Black,
			   sf::Color background_color = BG_COLOR,
			   sf::Color background_color_hover = BG_HOVER,
			   std::string const& font_name = BASE_FONT_NAME);

	void Update() override;
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(vec2i mouse_pos) override;
	bool onHoverIn(vec2i mouse_pos={0,0}) override;
	bool onHoverOut() override;

	void setPos(vec2 pos) override;
	void setSize(vec2 size) override;
	void setOrigin(vec2 origin) override;

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

class SquareButton : public TextButton {
public:
	SquareButton(const std::string& text_string, vec2 pos, float size, 
				unsigned int character_size = FontSize::NORMAL,
				sf::Color text_color = sf::Color::Black,
				sf::Color background_color = BG_COLOR,
				sf::Color background_color_hover = BG_HOVER,
				std::string const& font_name = BASE_FONT_NAME);
	void setPos(vec2 pos) override;
	bool onClick(vec2i mouse_pos) override;
private:
	void UpdateTextButton(bool set_params = true);
	float side_size;
};

/*
Button for controls selection
*/
class ControlsTextButton : public TextButton
{
public:
	ControlsTextButton()=default;
	ControlsTextButton(std::string const& text_string, vec2 pos, float width,
					   unsigned int character_size = FontSize::NORMAL,
					   sf::Color text_color = sf::Color::Black,
					   sf::Color background_color = BG_COLOR,
					   sf::Color background_color_hover = BG_HOVER,
					   std::string const& font_name=BASE_FONT_NAME);

	bool onClick(vec2i mouse_pos) override;
	bool onHoverIn(vec2i mouse_pos={0,0}) override;
	bool onHoverOut() override;
	bool onKeyType(sf::Event::KeyEvent e) override;

	void setActive(bool active) override;
	void setKey(sf::Keyboard::Key key) { this->key = key; }
	sf::Keyboard::Key* getKeyRef() { return &key; }

private:
	bool UpdateControlsTextButton(bool set_params = true);

	sf::Keyboard::Key key;
};

class WorldSelectButton : public TextButton
{
public:
	WorldSelectButton()=default;
	WorldSelectButton(std::string const& text_string, vec2 pos, float width,
					   unsigned int character_size = FontSize::NORMAL,
					   sf::Color text_color = sf::Color::Black,
					   sf::Color background_color = BG_COLOR,
					   sf::Color background_color_hover = BG_HOVER,
					   std::string const& font_name=BASE_FONT_NAME);

	bool onClick(vec2i mouse_pos) override;

	void setWorldName(std::string name) { world_name = name; }
private:
	std::string world_name;
};

class Tooltip : public GUIObject {
public:
	Tooltip()=default;
	Tooltip(std::string const& text_string, sf::Time show_after);


	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=true) override;

	void StartTimer(vec2i mouse_pos);
	void StopTimer();

	void setMousePos(vec2i mouse_pos);

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
	Checkbox(bool active, vec2 pos, vec2 size ={40, 40},
			 sf::Color background_color = BG_COLOR, sf::Color background_color_hover = BG_HOVER );

	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(vec2i mouse_pos) override;
	bool onHoverIn(vec2i mouse_pos={0,0}) override;
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
	Slider(vec2 pos, float width, float start_value, float min_value, float max_value,
		   sf::Color background_color = BG_COLOR, sf::Color background_color_hover = BG_HOVER );

	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(vec2i mouse_pos) override;
	bool onHoverIn(vec2i mouse_pos={0,0}) override;
	bool onHoverOut() override;
	void UpdateClickDrag(vec2i mouse_pos);

	float* getValueRef() { return &value; }

private:
	void UpdateSlider();

	sf::RectangleShape bar_shape;
	sf::RectangleShape rect_shape;

	vec2 bar_pos;
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
	Scrollbar(vec2 pos, float height, float start_val, float min_value, float max_value,
			  sf::Color background_color = BG_COLOR, sf::Color background_color_hover = BG_HOVER
	);

	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(vec2i mouse_pos) override;
	bool onHoverIn(vec2i mouse_pos={0,0}) override;
	bool onHoverOut() override;
	void UpdateClickDrag(vec2i mouse_pos);

	float getValue() { return value; }
	float* getValueRef() { return &value; }

	void setMaxValue(float max_value) { this->max_value = max_value; }
	void setPos(vec2 pos) override;

	void UpdateScrollbar(float val, bool set_val = true);
private:

	sf::RectangleShape bar_shape;
	sf::RectangleShape rect_shape;

	vec2 bar_pos;
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
	ObjContainer(vec2 pos, vec2 size);

	~ObjContainer() { delete scrollbar; }

	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	void AddObject(GUIObject* obj);

	bool onClick(vec2i mouse_pos) override;
	bool onClickRelease() override;
	bool onHoverIn(vec2i mouse_pos={0,0}) override;
	bool onHoverOut() override;
	bool onMouseWheel(float delta) override;
	bool onKeyType(sf::Event::KeyEvent e) override;

	void UpdateClickDrag(vec2i mouse_pos) override;
	void UpdateHoveredMousePos(vec2i mouse_pos) override;

private:
	void UpdateObjContainer(bool set_params = true);

	std::vector<GUIObject*> gui_objects;
	Scrollbar* scrollbar;

	bool show_scrollbar = false;

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
	TextInputBox(vec2 pos, float width, unsigned int character_size = FontSize::SMALL);

	void Update() override;
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(vec2i mouse_pos) override;
	void UpdateClickDrag(vec2i mouse_pos);
	bool onKeyType(sf::Event::KeyEvent e) override;
	std::string* getStringRef() { return &text_string; }

private:
	void Init(); // sets the size (height) and the positions of stuff
	void UpdateText(); // updates the text's string and such
	void UpdateCursorPos();

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

class InvItemButton : public GUIObject {
public:
	InvItemButton()=default;
	InvItemButton( int item, vec2 pos, float width );

	void Update() override;
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(vec2i mouse_pos) override;
	bool onHoverIn(vec2i mouse_pos={0,0}) override;
	bool onHoverOut() override;

	void setSelected(bool selected);
	void setPos(vec2 pos) override;
	void setOrigin(vec2 origin) override;

	bool getSelected() { return selected; }
	int getItem() { return item; }

protected:
	virtual void Init();
	virtual void UpdateButtonParams();

	bool selected = false;

	sf::Font* font;
	uint character_size;
	float width = 0;
	float margin = 10.f;

	sf::RectangleShape rect_shape;
	sf::Sprite icon_sprite;
	sf::Text text_obj;

	sf::Color text_color;
	sf::Color background_color;
	sf::Color background_color_hover;
	sf::Color background_color_selected;
	Tweener color_tw;

	int item;
};

class InvToolButton : public InvItemButton
{
public:
	InvToolButton()=default; // probably can be removed
	InvToolButton(int item, vec2 pos, float width);

	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	void UpdateDurability();
private:
	void Init() override;
	void UpdateButtonParams() override;

	float bar_height = 3.f;
	sf::RectangleShape bar_shape;
	bool tool_is_bowl = false;
};

class InvRecipeButton : public InvItemButton
{
public:
	InvRecipeButton(Item::Recipe recipe, vec2 pos, float width, ButtonActionImpl* button_action_impl);
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;
	void setCraftable(bool craftable);
	Item::Recipe getItemRecipe() { return recipe; }
	bool onClick(vec2i pos) override;
private:
	void Init() override;
	void UpdateButtonParams() override;

	sf::Sprite craftable_sprite;
	bool craftable = false;
	Item::Recipe recipe;
};
	
class InvActionButton : public TextButton
{
public:
	InvActionButton()=default;
	InvActionButton(std::string const& text_string, int item, vec2 pos, float width,
					   unsigned int character_size = FontSize::NORMAL,
					   sf::Color text_color = INV_TEXT_COLOR,
					   sf::Color background_color = INV_ACCENT_COLOR,
					   sf::Color background_color_hover = INV_ACCENT_COLOR2,
					   std::string const& font_name=BASE_FONT_NAME);

	bool onClick(vec2i mouse_pos) override;

private:
	int item;
};

class InvPageButton : public GUIObject
{
public:
	InvPageButton()=default;
	InvPageButton(
		vec2 pos,
		const std::string& texture_name,
		vec2 pos_in_texture
	);

	void Update() override;
	void Render(sf::RenderTarget& target,
				sf::RenderTarget& tooltip_render_target,
				bool draw_on_tooltip_render_target=false) override;

	bool onClick(vec2i mouse_pos) override;
	bool onHoverIn(vec2i mouse_pos={0,0}) override;
	bool onHoverOut() override;

	void setSelected(bool selected);
	void setPos(vec2 pos) override;
	void setOrigin(vec2 origin) override;

	static float margin;
	static float side_size;
private:
	void Init();
	void UpdateButtonParams();

	bool selected = false;
	std::string texture_name;
	vec2 pos_in_texture;

	sf::RectangleShape rect_shape;
	sf::Sprite sprite;

	sf::Color background_color = INV_ACCENT_COLOR2;
	sf::Color background_color_hover = INV_ACCENT_COLOR;
	sf::Color selected_color = INV_WINDOW_COLOR;
	Tweener color_tw;
};