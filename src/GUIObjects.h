#pragma once

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "Game.h"

// Forward declaration
class Tooltip;

class ButtonActionImpl {
public:
	ButtonActionImpl(Game& game);

	void test();

	Game& game;
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
	virtual void Render(sf::RenderTarget& target);

	virtual void onClick(ButtonActionImpl& impl) {}
	virtual void onHoverIn(sf::Vector2i mouse_pos={ 0,0 });
	virtual void onHoverOut();

	virtual void UpdateHoveredMousePos(sf::Vector2i mouse_pos); // when mouse has moved on top of object

	bool isMouseIn(sf::Vector2i mouse_pos);

	virtual void setPos(sf::Vector2f pos) { this->pos = pos; }
	virtual void setSize(sf::Vector2f size) { this->size = size; }
	virtual void setTooltip(Tooltip* tooltip) { this->tooltip = tooltip; }

	sf::Vector2f getPos() { return pos; }
	sf::Vector2f getSize() { return size; }
	bool getHovered() { return hovered; }

protected:
	sf::Vector2f pos;
	sf::Vector2f size;

	bool hovered = false;

	Tooltip* tooltip = nullptr;

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

	void Render(sf::RenderTarget& target) override;


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
			   sf::Vector2f pos, float margin,
			   unsigned int character_size = FontSize::NORMAL,
			   sf::Color text_color = sf::Color::Black,
			   sf::Color background_color = sf::Color(160, 160, 160),
			   sf::Color background_color_hover = sf::Color(100, 100, 100),
			   std::string const& font_name=BASE_FONT_NAME );


	void Update() override;
	void Render(sf::RenderTarget& target) override;

	void onClick(ButtonActionImpl& impl) override;
	void onHoverIn(sf::Vector2i mouse_pos={ 0,0 }) override;
	void onHoverOut() override;

	void setPos(sf::Vector2f pos) override;

private:
	void UpdateTextButton(bool set_params = true);

	sf::Font* font;
	unsigned int character_size;
	float margin;

	sf::Color text_color;
	sf::Color background_color;
	sf::Color background_color_hover;

	std::string text_string;
	sf::Text text_obj;
	sf::RectangleShape rect_shape;
};

class Tooltip : public GUIObject {
public:
	Tooltip()=default;
	Tooltip(std::string const& text_string, sf::Time show_after);
	

	void Update() override;
	void Render(sf::RenderTarget& target) override;

	void StartTimer(sf::Vector2i mouse_pos);
	void StopTimer();

	void setMousePos(sf::Vector2i mouse_pos);

private:
	TextBox text_box;
	sf::RectangleShape rect_shape;

	bool timer_active = false;
	sf::Clock clock;
	sf::Time show_after;
};
