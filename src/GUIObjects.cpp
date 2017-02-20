#include "GUIObjects.h"

#include "ResourceManager.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <iostream>
using namespace std;

GUIObject::GUIObject(sf::Vector2f pos, sf::Vector2f size) : pos(pos), size(size) { }

GUIObject::~GUIObject() { }

ButtonActionImpl::ButtonActionImpl(Game & game) : game(game)
{
}

void ButtonActionImpl::test()
{
	game.ChangeActiveState(State::Game, State::MainMenu);
}


bool GUIObject::isMouseIn(sf::Vector2i mouse_pos)
{
	if (mouse_pos.x >= pos.x && mouse_pos.x <= pos.x + size.x) {
		if (mouse_pos.y >= pos.y && mouse_pos.y <= pos.y + size.y) {
			return true;
		}
	}
	return false;
}

TextBox::TextBox(std::string const& text_string, sf::Vector2f pos, float width, sf::Font * font, sf::Color color, unsigned int character_size) :
	GUIObject(pos, sf::Vector2f(width, 0)), font(font), color(color), character_size(character_size), text_string(text_string), original_text_string(text_string)
{
	UpdateTextBox();
}

TextBox::TextBox(std::string const& text_string, sf::Vector2f pos, float width, std::string const& font_name, sf::Color color, unsigned int character_size) :
	GUIObject(pos, sf::Vector2f(width, 0)), font(&ResourceManager::getFont(font_name)), color(color), character_size(character_size), text_string(text_string), original_text_string(text_string)
{
	UpdateTextBox();
}

void TextBox::Render(sf::RenderTarget & target)
{
	target.draw(text_obj);
}

void TextBox::setPos(sf::Vector2f pos, bool update)
{
	this->pos = pos;
	if (update) UpdateTextBox();
}

void TextBox::setWidth(float width, bool update)
{
	size.x = width;
	if (update) UpdateTextBox();
}

void TextBox::setFont(sf::Font* font, bool update)
{
	this->font = font;
	if (update) UpdateTextBox();
}

void TextBox::setFont(std::string const& font_name, bool update)
{
	this->font = &ResourceManager::getFont(font_name);
	if (update) UpdateTextBox();
}

void TextBox::setColor(sf::Color color, bool update)
{
	this->color = color;
	if (update) UpdateTextBox();
}

void TextBox::setCharacterSize(unsigned int character_size, bool update)
{
	this->character_size = character_size;
	if (update) UpdateTextBox();
}

void TextBox::setTextString(std::string const& text_string, bool update)
{
	this->text_string = text_string;
	if (update) UpdateTextBox();
}

void TextBox::UpdateTextBox(bool set_text_obj_params, int start_at_index)
{
	if (set_text_obj_params) {
		text_obj.setPosition(pos);
		text_obj.setFont(*font);
		text_obj.setCharacterSize(character_size);
		text_string = original_text_string;
	}

	text_obj.setString(text_string);

	if (text_obj.getLocalBounds().width <= size.x) {
		setSize(sf::Vector2f(text_obj.getLocalBounds().width, text_obj.getLocalBounds().height));
		return;
	}

	int last_space = -1;
	bool changed = false;
	int i, len;

	for (i = start_at_index, len = text_string.size(); i != len; ++i) {
		if (text_string[i] == ' ') {
			last_space = i;
			continue;
		}
		float x = text_obj.findCharacterPos(i).x;
		if (x >= pos.x + size.x && last_space != -1) {
			text_string[last_space] = '\n';
			changed = true;
			break;
		}
	}

	if (changed) UpdateTextBox(false, i);
	else {
		setSize(sf::Vector2f(text_obj.getLocalBounds().width, text_obj.getLocalBounds().height));
	}
}

TextButton::TextButton(std::string const & text_string,
					   sf::Vector2f pos,
					   float margin,
					   unsigned int character_size,
					   sf::Color text_color,
					   sf::Color background_color,
					   sf::Color background_color_hover,
					   std::string const & font_name) :
	GUIObject(pos, { 0,0 }),
	text_string(text_string),
	character_size(character_size),
	margin(margin),
	text_color(text_color),
	background_color(background_color),
	background_color_hover(background_color_hover),
	font(&ResourceManager::getFont(font_name))
{
	UpdateTextButton();
}

void TextButton::Render(sf::RenderTarget & target)
{
	target.draw(rect_shape);
	target.draw(text_obj);
}

void TextButton::onClick(ButtonActionImpl& impl)
{
	impl.test();
}

void TextButton::onHoverIn()
{
	hovered = true;
	rect_shape.setFillColor(background_color_hover);
}

void TextButton::onHoverOut()
{
	hovered = false;
	rect_shape.setFillColor(background_color);
}

void TextButton::setPos(sf::Vector2f pos)
{
	this->pos = pos;
	UpdateTextButton(false);
}

void TextButton::UpdateTextButton(bool set_params)
{
	if (set_params) {
		text_obj.setString(text_string);
		text_obj.setFont(*font);
		text_obj.setCharacterSize(character_size);
		text_obj.setFillColor(text_color);
	}

	auto text_rect = text_obj.getLocalBounds();
	text_obj.setOrigin(text_rect.width / 2.f, text_rect.height);
	text_obj.setPosition(sf::Vector2f(pos.x + text_rect.width / 2.f + margin, pos.y + margin*2.f));

	rect_shape.setPosition(pos);
	rect_shape.setSize(sf::Vector2f(text_rect.width + margin*2.f, text_rect.height + margin*2.f));
	rect_shape.setFillColor(background_color);

	setSize(rect_shape.getSize());
}
