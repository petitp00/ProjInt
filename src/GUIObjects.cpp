#include "GUIObjects.h"

#include "ResourceManager.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <iostream>
using namespace std;

ButtonActionImpl::ButtonActionImpl(Game & game) : game(game)
{
}

GUIObject::GUIObject(sf::Vector2f pos, sf::Vector2f size) : pos(pos), size(size) { }

GUIObject::~GUIObject()
{
	if (tooltip) delete tooltip;
	if (action) delete action;
}

void GUIObject::Update()
{
	if (tooltip) tooltip->Update();
}

void GUIObject::Render(sf::RenderTarget & target)
{
	if (tooltip) tooltip->Render(target);
}

void GUIObject::onHoverIn(sf::Vector2i mouse_pos)
{
	hovered = true;
	if (tooltip) tooltip->StartTimer(mouse_pos);
}

void GUIObject::onHoverOut()
{
	hovered = false;
	if (tooltip) tooltip->StopTimer();
}

void GUIObject::UpdateHoveredMousePos(sf::Vector2i mouse_pos)
{
	if (tooltip) tooltip->setMousePos(mouse_pos);
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
	GUIObject::Render(target);
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
		text_obj.setFillColor(color);
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

void TextButton::Update()
{
	GUIObject::Update();
}

void TextButton::Render(sf::RenderTarget & target)
{
	target.draw(rect_shape);
	target.draw(text_obj);

	GUIObject::Render(target);
}

void TextButton::onClick()
{
	onHoverOut();
	if (action) 
		(*action)(button_action_impl);
	else { cout << "w" << endl; }
}

void TextButton::onHoverIn(sf::Vector2i mouse_pos)
{
	GUIObject::onHoverIn(mouse_pos);
	rect_shape.setFillColor(background_color_hover);
}

void TextButton::onHoverOut()
{
	GUIObject::onHoverOut();
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

Tooltip::Tooltip(std::string const & text_string, sf::Time show_after) :
	text_box(text_string, sf::Vector2f(0, 0), 200, BASE_FONT_NAME, sf::Color::Black, FontSize::TINY),
	show_after(show_after)
{
	rect_shape.setSize(text_box.getSize() + sf::Vector2f(20.f, 20.f));
	rect_shape.setFillColor(sf::Color::White);

	size = rect_shape.getSize();
}

void Tooltip::Update()
{
}

void Tooltip::Render(sf::RenderTarget & target)
{
	if (timer_active && clock.getElapsedTime() >= show_after) {
		target.draw(rect_shape);
		text_box.Render(target);
	}
}

void Tooltip::StartTimer(sf::Vector2i mouse_pos)
{
	timer_active = true;
	clock.restart();

	setMousePos(mouse_pos);
}

void Tooltip::StopTimer()
{
	timer_active = false;
}

void Tooltip::setMousePos(sf::Vector2i mouse_pos)
{
	pos = sf::Vector2f(mouse_pos) + sf::Vector2f(0.f, size.y*1.0f) - size/2.f;

	if (pos.x + size.x >= WINDOW_WIDTH - 10.f) pos.x = WINDOW_WIDTH - size.x - 10.f;
	else if (pos.x <= 10.f) pos.x = 10.f;
	if (pos.y <= 10.f) pos.y = 10.f;
	else if (pos.y + size.y >= WINDOW_HEIGHT - 10.f) pos.y = WINDOW_HEIGHT - size.y - 10.f; 

	rect_shape.setPosition(pos);
	text_box.setPos(pos + sf::Vector2f(10.f, 5.f));
}
