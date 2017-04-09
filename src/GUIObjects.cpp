#include "GUIObjects.h"

#include "ResourceManager.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "MenuState.h"

#include <iostream>
using namespace std;

ButtonActionImpl::ButtonActionImpl(Game* game, MenuState* menu_state, GameState* game_state) : game(game), menu_state(menu_state), game_state(game_state) {}

GUIObject::GUIObject(sf::Vector2f pos, sf::Vector2f size) : pos(pos), size(size) {}

GUIObject::~GUIObject() {
	if (tooltip) delete tooltip;
	if (action) delete action;
}

void GUIObject::Update() {
	if (tooltip) tooltip->Update();
}

void GUIObject::Render(sf::RenderTarget & target, sf::RenderTarget& tooltip_render_target, bool draw_on_tooltip_render_target) {
	if (tooltip) tooltip->Render(target, tooltip_render_target);
}

bool GUIObject::onHoverIn(sf::Vector2i mouse_pos) {
	hovered = true;
	if (tooltip) tooltip->StartTimer(mouse_pos);
	return true;
}

bool GUIObject::onHoverOut() {
	hovered = false;
	if (tooltip) tooltip->StopTimer();
	return true;
}

void GUIObject::UpdateHoveredMousePos(sf::Vector2i mouse_pos) {
	if (tooltip) tooltip->setMousePos(mouse_pos);
}

bool GUIObject::isMouseIn(sf::Vector2i mouse_pos) {
	if (mouse_pos.x >= pos.x - origin.x && mouse_pos.x <= pos.x + size.x - origin.x) {
		if (mouse_pos.y >= pos.y - origin.y && mouse_pos.y <= pos.y + size.y - origin.y) {
			return true;
		}
	}
	return false;
}

// TEXTBOX //

TextBox::TextBox(std::string const& text_string, sf::Vector2f pos, float width, sf::Font * font, sf::Color color, unsigned int character_size) :
	GUIObject(pos, sf::Vector2f(width, 0)), font(font), color(color),
	character_size(character_size), text_string(text_string), original_text_string(text_string), width(width) {
	size.x = width;
	UpdateTextBox();
}

TextBox::TextBox(std::string const& text_string, sf::Vector2f pos, float width, std::string const& font_name, sf::Color color, unsigned int character_size) :
	GUIObject(pos, sf::Vector2f(width, 0)), font(&ResourceManager::getFont(font_name)), color(color), character_size(character_size), text_string(text_string), original_text_string(text_string), width(width) {
	size.x = width;
	UpdateTextBox();
}

void TextBox::Render(sf::RenderTarget & target, sf::RenderTarget& tooltip_render_target, bool draw_on_tooltip_render_target) {
	if (!draw_on_tooltip_render_target)
		target.draw(text_obj);
	else
		tooltip_render_target.draw(text_obj);
	GUIObject::Render(target, tooltip_render_target);
}

void TextBox::setPos(sf::Vector2f pos, bool update) {
	this->pos = pos;
	if (update) UpdateTextBox();
}

void TextBox::setOrigin(sf::Vector2f origin)
{
	this->origin = origin;
	text_obj.setOrigin(origin);
}

void TextBox::setWidth(float width, bool update) {
	size.x = width;
	if (update) UpdateTextBox();
}

void TextBox::setFont(sf::Font* font, bool update) {
	this->font = font;
	if (update) UpdateTextBox();
}

void TextBox::setFont(std::string const& font_name, bool update) {
	this->font = &ResourceManager::getFont(font_name);
	if (update) UpdateTextBox();
}

void TextBox::setColor(sf::Color color, bool update) {
	this->color = color;
	if (update) UpdateTextBox();
}

void TextBox::setCharacterSize(unsigned int character_size, bool update) {
	this->character_size = character_size;
	if (update) UpdateTextBox();
}

void TextBox::setTextString(std::string const& text_string, bool update) {
	original_text_string = text_string;
	this->text_string = text_string;
	if (update) UpdateTextBox();
}

void TextBox::UpdateTextBox(bool set_text_obj_params, int start_at_index) {
	if (set_text_obj_params) {
		text_obj.setPosition(pos);
		text_obj.setFont(*font);
		text_obj.setCharacterSize(character_size);
		text_obj.setFillColor(color);
		text_string = original_text_string;
	}

	text_obj.setString(text_string);

	if (text_obj.getLocalBounds().width <= width) {
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
		if (x >= pos.x + width - origin.x && last_space != -1) {
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
					   float width,
					   unsigned int character_size,
					   sf::Color text_color,
					   sf::Color background_color,
					   sf::Color background_color_hover,
					   std::string const & font_name) :
	GUIObject(pos, {0,0}),
	text_string(text_string),
	character_size(character_size),
	width(width),
	text_color(text_color),
	background_color(background_color),
	background_color_hover(background_color_hover),
	font(&ResourceManager::getFont(font_name)) {
	color_tw.Reset(TweenType::QuartInOut, 0, 0, sf::milliseconds(50));
	UpdateTextButton();
}

void TextButton::Update() {
	GUIObject::Update();

	rect_shape.setFillColor(LerpColor(background_color, background_color_hover, color_tw.Tween()));
}

void TextButton::Render(sf::RenderTarget & target, sf::RenderTarget& tooltip_render_target, bool draw_on_tooltip_render_target) {
	target.draw(rect_shape);
	target.draw(text_obj);

	GUIObject::Render(target, tooltip_render_target);
}

bool TextButton::onClick(sf::Vector2i mouse_pos) {
	onHoverOut();
	if (action) {
		(*action)(button_action_impl);
		return true;
	}
	return false;
}

bool TextButton::onHoverIn(sf::Vector2i mouse_pos) {
	GUIObject::onHoverIn(mouse_pos);
	color_tw.Reset(TweenType::QuartInOut, 0, 1, sf::milliseconds(100));
	return false;
}

bool TextButton::onHoverOut() {
	GUIObject::onHoverOut();
	color_tw.Reset(TweenType::QuartInOut, 1, 0, sf::milliseconds(100));
	return false;
}

void TextButton::setPos(sf::Vector2f pos) {
	this->pos = pos;
	UpdateTextButton(false);
}

void TextButton::setOrigin(sf::Vector2f origin) {
	GUIObject::setOrigin(origin);
	UpdateTextButton(false);
}

void TextButton::UpdateTextButton(bool set_params) {
	static float hhh;
	if (set_params) {
		std::string upper_string;
		for (auto c : text_string) {
			upper_string += toupper('o');
		}
		text_obj.setString(upper_string);

		text_obj.setFont(*font);
		text_obj.setCharacterSize(character_size);
		text_obj.setFillColor(text_color);

		hhh = text_obj.getLocalBounds().height;
	}

	auto text_rect = text_obj.getLocalBounds();
	float height;
	if (set_params) height = text_rect.height;
	else height = hhh;

	if (width == 0.f) {
		rect_shape.setSize(sf::Vector2f(text_rect.width + margin*2.f, height + margin*2.f));

		text_obj.setOrigin(text_rect.width / 2.f + origin.x, height + origin.y);
		text_obj.setPosition(sf::Vector2f(pos.x + text_rect.width / 2.f + margin, pos.y + rect_shape.getLocalBounds().height/2.f));
	}
	else {
		rect_shape.setSize(sf::Vector2f(width, height + margin*2.f));

		text_obj.setOrigin(origin.x, height + origin.y);
		text_obj.setPosition(sf::Vector2f(pos.x + margin, pos.y + rect_shape.getLocalBounds().height/2.f));
	}

	text_obj.setString(text_string);

	rect_shape.setOrigin(origin);
	rect_shape.setPosition(pos);
	rect_shape.setFillColor(background_color);
	setSize(rect_shape.getSize());
}

// CONTROLS TEXT BUTTON
ControlsTextButton::ControlsTextButton(std::string const & text_string, sf::Vector2f pos, float width,
									   unsigned int character_size, sf::Color text_color, sf::Color background_color,
									   sf::Color background_color_hover, std::string const & font_name) :
	TextButton(text_string, pos, width, character_size, text_color, background_color, background_color_hover, font_name) {
	UpdateControlsTextButton();
}

bool ControlsTextButton::onClick(sf::Vector2i mouse_pos) {
	if (!active) {
		active = true;
		return true;
	}
	return false;
}

bool ControlsTextButton::onHoverIn(sf::Vector2i mouse_pos) {
	if (!active) TextButton::onHoverIn(mouse_pos);
	hovered = true;
	return false;
}

bool ControlsTextButton::onHoverOut() {
	if (!active) TextButton::onHoverOut();
	hovered = false;
	return false;
}

bool ControlsTextButton::onKeyType(sf::Event::KeyEvent e)
{
	text_string = getKeyString(e.code);
	this->key = e.code;
	setActive(false);
	if (action) (*action)(button_action_impl);
	if (UpdateControlsTextButton()) {
		return true;
	}
	return false;
}

void ControlsTextButton::setActive(bool active) {
	GUIObject::setActive(active);
	if (!active) onHoverOut();
}

bool ControlsTextButton::UpdateControlsTextButton(bool set_params) {
	UpdateTextButton(set_params);
	return true; //TODO: Check if key entered is ok, else return false
}

// WORLDSELECTBUTTON //
WorldSelectButton::WorldSelectButton(std::string const & text_string, sf::Vector2f pos, float width, unsigned int character_size, sf::Color text_color, sf::Color background_color, sf::Color background_color_hover, std::string const & font_name) :
	TextButton(text_string, pos, width, character_size, text_color, background_color, background_color_hover, font_name)
{
}

bool WorldSelectButton::onClick(sf::Vector2i mouse_pos)
{
	onHoverOut();
	if (action) {
		button_action_impl->load_world_name = world_name;
		(*action)(button_action_impl);
		return true;
	}
	return false;
}

// TOOLTIP //
Tooltip::Tooltip(std::string const & text_string, sf::Time show_after) :
	text_box(text_string, sf::Vector2f(0, 0), 300, BASE_FONT_NAME, sf::Color::Black, FontSize::TINY),
	show_after(show_after) {
	rect_shape.setSize(text_box.getSize() + sf::Vector2f(20.f, 20.f));
	rect_shape.setFillColor(sf::Color::White);

	size = rect_shape.getSize();
}

void Tooltip::Update() {
}

void Tooltip::Render(sf::RenderTarget & target, sf::RenderTarget& tooltip_render_target, bool draw_on_tooltip_render_target) {
	if (timer_active && clock.getElapsedTime() >= show_after) {
		if (!alpha_tweener_started) {
			alpha_tweener_started = true;
			alpha_tweener.Reset(TweenType::QuartInOut, 0, 255, sf::milliseconds(200));
		}
		rect_shape.setFillColor(sf::Color(255, 255, 255, int(alpha_tweener.Tween())));
		text_box.setColor(sf::Color(0, 0, 0, int(alpha_tweener.Tween())));
		tooltip_render_target.draw(rect_shape);
		text_box.Render(target, tooltip_render_target, true);
	}
}

void Tooltip::StartTimer(sf::Vector2i mouse_pos) {
	timer_active = true;
	clock.restart();

	setMousePos(mouse_pos);

}

void Tooltip::StopTimer() {
	timer_active = false;
	alpha_tweener_started = false;
}

void Tooltip::setMousePos(sf::Vector2i mouse_pos) {
	pos = sf::Vector2f(mouse_pos) + sf::Vector2f(0.f, size.y*1.0f) - size / 2.f;

	if (pos.x + size.x >= WINDOW_WIDTH - 10.f) pos.x = WINDOW_WIDTH - size.x - 10.f;
	else if (pos.x <= 10.f) pos.x = 10.f;
	if (pos.y <= 10.f) pos.y = 10.f;
	else if (pos.y + size.y >= WINDOW_HEIGHT - 10.f) pos.y = WINDOW_HEIGHT - size.y - 10.f;

	rect_shape.setPosition(pos);
	text_box.setPos(pos + sf::Vector2f(10.f, 5.f));
}

// CHECKBOX //

Checkbox::Checkbox(bool active, sf::Vector2f pos, sf::Vector2f size, sf::Color background_color, sf::Color background_color_hover) :
	GUIObject(pos, size),
	active(active),
	background_color(background_color),
	background_color_hover(background_color_hover) {
	text_obj.setFont(ResourceManager::getFont(BASE_FONT_NAME));
	text_obj.setCharacterSize(FontSize::SMALL);
	text_obj.setFillColor(sf::Color::Black);
	text_obj.setString("X");
	text_obj.setOrigin(text_obj.getLocalBounds().width / 2.f, text_obj.getLocalBounds().height);
	text_obj.setPosition(pos + size/2.f);

	if (!active) text_obj.setString("");

	color_tw.Reset(TweenType::QuartInOut, 0, 0, sf::milliseconds(50));
	UpdateCheckbox();
}

void Checkbox::Update() {
	GUIObject::Update();

	rect_shape.setFillColor(LerpColor(background_color, background_color_hover, color_tw.Tween()));
}

void Checkbox::Render(sf::RenderTarget & target, sf::RenderTarget & tooltip_render_target, bool draw_on_tooltip_render_target) {
	GUIObject::Render(target, tooltip_render_target, draw_on_tooltip_render_target);

	target.draw(rect_shape);
	target.draw(text_obj);
}

bool Checkbox::onClick(sf::Vector2i mouse_pos) {
	active = !active;
	if (!active) text_obj.setString("");
	else text_obj.setString("X");
	if (action) {
		(*action)(button_action_impl);
		return true;
	}
	return false;
}

bool Checkbox::onHoverIn(sf::Vector2i mouse_pos) {
	GUIObject::onHoverIn(mouse_pos);
	color_tw.Reset(TweenType::QuartInOut, 0, 1, sf::milliseconds(100));
	return false;
}

bool Checkbox::onHoverOut() {
	GUIObject::onHoverOut();
	color_tw.Reset(TweenType::QuartInOut, 1, 0, sf::milliseconds(100));
	return false;
}

void Checkbox::UpdateCheckbox() {
	rect_shape.setPosition(pos);
	rect_shape.setSize(size);
}

Slider::Slider(sf::Vector2f pos, float width, float start_value, float min_value, float max_value, sf::Color background_color, sf::Color background_color_hover) :
	GUIObject(pos, {40,40}),
	bar_width(width),
	bar_pos(pos - sf::Vector2f(0, 15/2.f - 20)),
	value(start_value),
	min_value(min_value),
	max_value(max_value),
	background_color(background_color),
	background_color_hover(background_color_hover) {
	pos.x = pos.x + ((start_value - min_value) / max_value) * width;
	color_tw.Reset(TweenType::QuartInOut, 0, 0, sf::milliseconds(50));
	UpdateSlider();
}

void Slider::Update() {
	rect_shape.setFillColor(LerpColor(background_color, background_color_hover, color_tw.Tween()));
}

void Slider::Render(sf::RenderTarget & target, sf::RenderTarget & tooltip_render_target, bool draw_on_tooltip_render_target) {
	target.draw(bar_shape);
	target.draw(rect_shape);
}

bool Slider::onClick(sf::Vector2i mouse_pos) {
	GUIObject::onClick(mouse_pos);
	return true;
}

bool Slider::onHoverIn(sf::Vector2i mouse_pos) {
	GUIObject::onHoverIn(mouse_pos);
	color_tw.Reset(TweenType::QuartInOut, 0, 1, sf::milliseconds(100));
	return false;
}

bool Slider::onHoverOut() {
	GUIObject::onHoverOut();
	color_tw.Reset(TweenType::QuartInOut, 1, 0, sf::milliseconds(100));
	return false;
}

void Slider::UpdateClickDrag(sf::Vector2i mouse_pos) {
	float mx = float(mouse_pos.x);
	pos.x = min(max(bar_pos.x, mx - 20), bar_pos.x + bar_width - 40.f);
	value = min_value + (pos.x - bar_pos.x) / (bar_width - 40) * max_value;
	rect_shape.setPosition(pos);
	if (action) (*action)(button_action_impl);
}

void Slider::UpdateSlider() {
	bar_shape.setSize({bar_width, 15});
	bar_shape.setPosition(bar_pos);
	bar_shape.setFillColor(sf::Color::Black);

	pos.x = bar_pos.x + (value-min_value)/max_value * (bar_width-40);

	rect_shape.setSize(sf::Vector2f(40, 40));
	rect_shape.setPosition(pos);
}

Scrollbar::Scrollbar(sf::Vector2f pos, float height, float start_val, float min_value, float max_value, sf::Color background_color, sf::Color background_color_hover) :
	GUIObject(pos, {15, 30}),
	bar_pos(pos),
	bar_height(height),
	value(start_val),
	min_value(min_value),
	max_value(max_value),
	background_color(background_color),
	background_color_hover(background_color_hover)

{
	pos.x = pos.x + ((start_val - min_value) / max_value) * height;
	color_tw.Reset(TweenType::QuartInOut, 0, 0, sf::milliseconds(50));

	bar_shape.setPosition(pos);
	bar_shape.setSize(sf::Vector2f(15, bar_height));
	bar_shape.setFillColor(sf::Color::Black);

	UpdateScrollbar(value);
}

void Scrollbar::Update() {
	rect_shape.setFillColor(LerpColor(background_color, background_color_hover, color_tw.Tween()));
}

void Scrollbar::Render(sf::RenderTarget & target, sf::RenderTarget & tooltip_render_target, bool draw_on_tooltip_render_target) {
	target.draw(bar_shape);
	target.draw(rect_shape);
}

bool Scrollbar::onClick(sf::Vector2i mouse_pos) {
	GUIObject::onClick(mouse_pos);
	return true;
}

bool Scrollbar::onHoverIn(sf::Vector2i mouse_pos) {
	GUIObject::onHoverIn(mouse_pos);
	color_tw.Reset(TweenType::QuartInOut, 0, 1, sf::milliseconds(100));
	return false;
}

bool Scrollbar::onHoverOut() {
	GUIObject::onHoverOut();
	color_tw.Reset(TweenType::QuartInOut, 1, 0, sf::milliseconds(100));
	return false;
}

void Scrollbar::UpdateClickDrag(sf::Vector2i mouse_pos) {
	float my = float(mouse_pos.y);
	pos.y = min(max(bar_pos.y, my - 15), bar_pos.y + bar_height - 30.f);
	value = min_value + (pos.y - bar_pos.y) / (bar_height - 30) * max_value;
	rect_shape.setPosition(pos);
	if (action) (*action)(button_action_impl);
}

void Scrollbar::UpdateScrollbar(float val) {
	value = val;
	pos.y = bar_pos.y + (value-min_value)/max_value * (bar_height-30);
	rect_shape.setPosition(pos);
	rect_shape.setSize(size);
}

/*
	OBJCONTAINER
*/

ObjContainer::ObjContainer(sf::Vector2f pos, sf::Vector2f size) :
	GUIObject(pos, size) {
	UpdateObjContainer();
	scrollbar = new Scrollbar(pos+sf::Vector2f(size.x, 10) - sf::Vector2f{40, 0}, size.y-30, 0, 0, max_offset);
}

void ObjContainer::Update() {
	for (auto o : gui_objects) {
		o->Update();
	}

	scrollbar->Update();
}

void ObjContainer::Render(sf::RenderTarget & target, sf::RenderTarget & tooltip_render_target, bool draw_on_tooltip_render_target) {
	render_texture.clear();
	render_texture.draw(rect_shape);

	for (auto o : gui_objects) {
		o->Render(render_texture, tooltip_render_target, draw_on_tooltip_render_target);
	}

	render_texture.display();
	target.draw(render_sprite);
	if (show_scrollbar)
		scrollbar->Render(target, tooltip_render_target, draw_on_tooltip_render_target);
}

void ObjContainer::AddObject(GUIObject * obj) {
	gui_objects.push_back(obj);

	if (obj->getPos().y + obj->getSize().y + 20 > max_offset +size.y - 20) {
		show_scrollbar = true;
		max_offset = obj->getPos().y + obj->getSize().y - size.y + 40.f;
		scrollbar->setMaxValue(max_offset);
	}
}

bool ObjContainer::onClick(sf::Vector2i mouse_pos) {
	bool ret = false;
	GUIObject::onClick(mouse_pos);
	for (auto o : gui_objects) {
		if (o->getHovered()) {
			if (!o->getActive()) if (o->onClick(mouse_pos)) {
				ret = true; return true;
			}
		}
		else if (o->getActive()) o->setActive(false);
	}

	if (scrollbar->getHovered()) if (scrollbar->onClick(mouse_pos)) ret = true;

	return ret;
}

bool ObjContainer::onClickRelease() {
	GUIObject::onClickRelease();
	for (auto o : gui_objects) {
		if (o->isClicked()) {
			o->onClickRelease();
		}
	}
	if (scrollbar->isClicked()) {
		scrollbar->onClickRelease();
	}
	return false;
}

bool ObjContainer::onHoverIn(sf::Vector2i mouse_pos) {
	GUIObject::onHoverIn(mouse_pos);
	return false;
}

bool ObjContainer::onHoverOut() {
	GUIObject::onHoverOut();
	return false;
}

float thicc = 2;

bool ObjContainer::onMouseWheel(float delta) {
	float scroll_speed = 20;
	GUIObject::onMouseWheel(delta);

	y_offset -= delta * scroll_speed;

	if (y_offset > max_offset) { y_offset = max_offset; }
	if (y_offset < min_offset) { y_offset = min_offset; }

	scrollbar->UpdateScrollbar(y_offset);

	view.setCenter(sf::Vector2f(size.x/2.f, size.y/2.f + y_offset));
	rect_shape.setPosition(view.getCenter() - view.getSize()/2.f + sf::Vector2f{thicc, thicc});
	render_texture.setView(view);

	return true;
}

bool ObjContainer::onKeyType(sf::Event::KeyEvent e)
{
	bool ret = false;
	for (auto o : gui_objects) {
		if (o->getActive()) {
			if (o->onKeyType(e)) ret = true;
		}
	}
	return ret;
}

void ObjContainer::UpdateClickDrag(sf::Vector2i mouse_pos) {
}

void ObjContainer::UpdateHoveredMousePos(sf::Vector2i mouse_pos) {
	sf::Vector2i mouse = sf::Vector2i(mouse_pos)- sf::Vector2i(pos) + sf::Vector2i(0, int(y_offset));

	for (auto o : gui_objects) {
		if (o->isClicked()) {
			o->UpdateClickDrag(mouse);
		}
		if (o->isMouseIn(mouse)) {
			if (!o->getHovered()) {
				o->onHoverIn(mouse);
			}
			else {
				o->UpdateHoveredMousePos(mouse);
			}
		}
		else if (o->getHovered()) {
			o->onHoverOut();
		}
	}

	if (scrollbar->isClicked()) {
		scrollbar->UpdateClickDrag(mouse_pos);
		y_offset = scrollbar->getValue();
		onMouseWheel(0);
	}
	if (scrollbar->isMouseIn(mouse_pos)) {
		if (!scrollbar->getHovered()) {
			scrollbar->onHoverIn(mouse_pos);
		}
		else {
			scrollbar->UpdateHoveredMousePos(mouse_pos);
		}
	}
	else if (scrollbar->getHovered()) {
		scrollbar->onHoverOut();
	}
}

void ObjContainer::UpdateObjContainer(bool set_params) {
	view.reset({0,0,size.x, size.y});

	render_texture.create(int(size.x), int(size.y));
	render_texture.setView(view);
	render_sprite.setTexture(render_texture.getTexture());
	render_sprite.setPosition(pos);


	if (set_params) {
		rect_shape.setPosition({thicc, thicc});
		rect_shape.setSize(size - sf::Vector2f{thicc*2, thicc*2});
		rect_shape.setOutlineColor(sf::Color::Black);
		rect_shape.setOutlineThickness(thicc);
		rect_shape.setFillColor(sf::Color(151, 196, 198));
	}
}

TextInputBox::TextInputBox(sf::Vector2f pos, float width, unsigned int character_size) :
	GUIObject(pos, {width,0}), character_size(character_size)
{
	Init();
}

void TextInputBox::Update()
{
	GUIObject::Update();

	if (active) {
		if (clock.getElapsedTime() >= cursor_blink_time) {
			show_cursor = !show_cursor;
			clock.restart();
		}
	}
}

void TextInputBox::Render(sf::RenderTarget & target, sf::RenderTarget & tooltip_render_target, bool draw_on_tooltip_render_target)
{
	target.draw(rect_shape);
	target.draw(text_obj);
	if (active && show_cursor)
		target.draw(cursor_shape);

	GUIObject::Render(target, tooltip_render_target);
}

bool TextInputBox::onClick(sf::Vector2i mouse_pos)
{
	GUIObject::onClick(mouse_pos);

	if (!active) active = true;
	clock.restart();

	float cw = 18;

	if (text_string.size() >= 2) {
		cw = text_obj.findCharacterPos(1).x - text_obj.findCharacterPos(0).x;
	}

	for (int i = text_string.size(); i >= 0; --i) {
		float cx = text_obj.findCharacterPos(i).x;

		if (mouse_pos.x >= cx -cw/2.f) {
			cursor_pos = i;
			break;
		}
	}

	UpdateCursorPos();

	return false;
}

void TextInputBox::UpdateClickDrag(sf::Vector2i mouse_pos)
{
	float cw = 18;

	if (text_string.size() >= 2) {
		cw = text_obj.findCharacterPos(1).x - text_obj.findCharacterPos(0).x;
	}

	for (int i = text_string.size(); i >= 0; --i) {
		float cx = text_obj.findCharacterPos(i).x;

		if (mouse_pos.x >= cx -cw/2.f) {
			cursor_pos = i;
			break;
		}
	}

	UpdateCursorPos();
}

bool TextInputBox::onKeyType(sf::Event::KeyEvent e)
{
	if (active) {
		char c = getKeyChar(e);
		if (c != 0) {
			text_string.insert(cursor_pos, {c});
			++cursor_pos;
		}
		else if (e.code == sf::Keyboard::Space) {
			text_string.insert(cursor_pos, " ");
			++cursor_pos;
		}
		else if (e.code == sf::Keyboard::BackSpace) {
			if (text_string.size() && cursor_pos >0) {
				text_string = text_string.substr(0, cursor_pos-1) + text_string.substr(cursor_pos);
				--cursor_pos;
			}
		}
		else if (e.code == sf::Keyboard::Left) {
			if (cursor_pos>0) --cursor_pos;
		}
		else if (e.code == sf::Keyboard::Right) {
			if (cursor_pos < text_string.size()) ++cursor_pos;
		}

		UpdateText();
		UpdateCursorPos();

		if (e.code == sf::Keyboard::Return) {
			if (action) {
				(*action)(button_action_impl);
				return true;
			}
		}

		if (e.code != sf::Keyboard::Escape)
			return true;
	}
	return false;
}

void TextInputBox::Init()
{
	float margin = 10.f;
	text_obj.setString("SAMPLE TEXT FOR SIZE");
	text_obj.setFont(ResourceManager::getFont(BASE_FONT_NAME));
	text_obj.setCharacterSize(character_size);
	text_obj.setFillColor(sf::Color::Black);
	size.y = text_obj.getLocalBounds().height + margin*2;

	text_obj.setOrigin(0, text_obj.getLocalBounds().height);
	text_obj.setPosition(pos.x + margin, pos.y + size.y/2.f);
	text_obj.setString("");

	rect_shape.setFillColor(sf::Color::White);
	rect_shape.setPosition(pos);
	rect_shape.setSize(size);

	cursor_shape.setFillColor(sf::Color::Black);
	cursor_shape.setSize({2.f, size.y - 2.f*5.f});
	cursor_shape.setPosition({text_obj.getPosition().x, pos.y+ 5});
}

void TextInputBox::UpdateText()
{
	string old_string = text_obj.getString();
	text_obj.setString(text_string);
	if (text_string.size()) {
		if (text_obj.findCharacterPos(text_string.size()-1).x >= rect_shape.getPosition().x + rect_shape.getSize().x - 10.f - text_obj.getCharacterSize()) {
			text_string = old_string;
			text_obj.setString(text_string);
			--cursor_pos;
		}
	}
}

void TextInputBox::UpdateCursorPos()
{
	if (cursor_pos > text_string.size()) cursor_pos =text_string.size()-1;
	cursor_shape.setPosition({text_obj.findCharacterPos(cursor_pos).x, pos.y+ 5});
}

InvItemButton::InvItemButton(Item::any item, sf::Vector2f pos,
							 float width, unsigned int character_size,
							 sf::Color text_color, sf::Color background_color, sf::Color background_color_hover, sf::Color background_color_selected) :
	GUIObject(pos, {0,0}),
	item(item),
	width(width),
	character_size(character_size),
	text_color(text_color),
	background_color(background_color),
	background_color_hover(background_color_hover),
	background_color_selected(background_color_selected)
{
	Init();
	color_tw.Reset(TweenType::Linear, 1, 0, sf::milliseconds(1));
}

void InvItemButton::Update()
{
	GUIObject::Update();

	rect_shape.setFillColor(LerpColor(background_color, background_color_hover, color_tw.Tween()));
}

void InvItemButton::Render(sf::RenderTarget & target, sf::RenderTarget & tooltip_render_target, bool draw_on_tooltip_render_target)
{
	target.draw(rect_shape);
	target.draw(icon_sprite);
	target.draw(text_obj);

	GUIObject::Render(target, tooltip_render_target);
}

bool InvItemButton::onClick(sf::Vector2i mouse_pos)
{
	onHoverOut();
	setSelected(true);
	if (action) {
		(*action)(button_action_impl);
		return true;
	}
	return false;
}

bool InvItemButton::onHoverIn(sf::Vector2i mouse_pos)
{
	GUIObject::onHoverIn(mouse_pos);
	color_tw.Reset(TweenType::QuartInOut, 0, 1, sf::milliseconds(100));
	return false;
}

bool InvItemButton::onHoverOut()
{
	GUIObject::onHoverOut();
	color_tw.Reset(TweenType::QuartInOut, 1, 0, sf::milliseconds(100));
	return false;
}

void InvItemButton::setSelected(bool selected)
{
	this->selected = selected;
	if (selected) rect_shape.setOutlineColor(sf::Color::White);
	else rect_shape.setOutlineColor(sf::Color::Transparent);
}

void InvItemButton::setPos(sf::Vector2f pos)
{
	this->pos = pos;
	UpdateButtonParams();
}

void InvItemButton::setOrigin(sf::Vector2f origin)
{
	this->origin = origin;
	UpdateButtonParams();
}

void InvItemButton::Init()
{
	float icon_scale = 2.f;
	float icon_size = icon_scale*Item::items_texture_size;
	rect_shape.setFillColor(background_color);
	rect_shape.setSize({width, icon_size + 2*margin});
	rect_shape.setOutlineColor(sf::Color::Transparent);
	rect_shape.setOutlineThickness(3);

	size = {width, icon_size + 2*margin};

	float ts = Item::items_texture_size;

	icon_sprite.setTexture(ResourceManager::getTexture(Item::texture_map_file));
	icon_sprite.setTextureRect(sf::IntRect(int(ts * item.pos_in_texture_map.x), int(ts * item.pos_in_texture_map.y), int(ts), int(ts)));
	icon_sprite.setOrigin(origin);
	icon_sprite.setScale(icon_scale, icon_scale);

	text_obj.setFont(ResourceManager::getFont(BASE_FONT_NAME));
	text_obj.setCharacterSize(character_size);
	text_obj.setFillColor(text_color);
	text_obj.setString(item.name);
	
	UpdateButtonParams();
}

void InvItemButton::UpdateButtonParams()
{
	rect_shape.setPosition(pos);
	rect_shape.setOrigin(origin);

	icon_sprite.setPosition(pos.x + margin - origin.x, pos.y + margin - origin.y);

	text_obj.setPosition(pos.x + margin*5.f + icon_sprite.getLocalBounds().width - origin.x, pos.y + 6 - origin.y);
}

InvActionButton::InvActionButton(std::string const & text_string, Item::any item, sf::Vector2f pos, float width, unsigned int character_size,
								 sf::Color text_color, sf::Color background_color, sf::Color background_color_hover, std::string const & font_name) : 
	TextButton(text_string, pos, width, character_size, text_color, background_color, background_color_hover, font_name), item(item)
{
}

bool InvActionButton::onClick(sf::Vector2i mouse_pos)
{
	onHoverOut();
	if (action) {
		button_action_impl->item = item;
		(*action)(button_action_impl);
		return true;
	}
	return false;
}
