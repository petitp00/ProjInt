#include "Text.h"

#include "ResourceManager.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <iostream>
using namespace std;

TextBox::TextBox(std::string text_string, sf::Vector2f pos, sf::Vector2f size, sf::Font * font, sf::Color color, unsigned int character_size):
	pos(pos), size(size), font(font), color(color), character_size(character_size), text_string(text_string), original_text_string(text_string)
{
	UpdateTextBox();
}

TextBox::TextBox(std::string text_string, sf::Vector2f pos, sf::Vector2f size, std::string const& font_name, sf::Color color, unsigned int character_size):
	pos(pos), size(size), font(&ResourceManager::getFont(font_name)), color(color), character_size(character_size), text_string(text_string), original_text_string(text_string)
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

void TextBox::setSize(sf::Vector2f size, bool update)
{
	this->size = size;
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

	if (text_obj.getLocalBounds().width <= size.x) { return; }

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
}

