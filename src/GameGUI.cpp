#include "GameGUI.h"
#include "Game.h"
#include "GUIObjects.h"

#include <iostream>
using namespace std;

static bool go_up_then_not_active = false;

static float INV_WINDOW_WIDTH = WINDOW_WIDTH / 4.f * 3;
static float INV_WINDOW_HEIGHT = WINDOW_HEIGHT / 4.f * 3;

static float WINDOW_X = (WINDOW_WIDTH - INV_WINDOW_WIDTH)/2.f;
static float WINDOW_STARTY = -INV_WINDOW_HEIGHT;
static float WINDOW_ENDY = (WINDOW_HEIGHT - INV_WINDOW_HEIGHT)/2.f;

static float margin_sides = 20.f;
static float margin_middle = 10.f;

Inventory::Inventory(Controls* controls) : controls(controls)
{
}

Inventory::~Inventory()
{
	for (int i = 0; i != gui_objects.size(); ++i) {
		delete gui_objects[i];
	}
}

void Inventory::Init()
{
	inv_buttons.clear();
	for (int i = 0; i != gui_objects.size(); ++i) {
		delete gui_objects[i];
	}
	gui_objects.clear();

	window_shape.setSize({INV_WINDOW_WIDTH, INV_WINDOW_HEIGHT});
	window_shape.setFillColor(INV_WINDOW_COLOR);

	auto ms = margin_sides;
	auto mm = margin_middle;

	// item desc
	item_desc_shape.setSize({INV_WINDOW_WIDTH/2.f - mm*2.f - ms*2.f, INV_WINDOW_HEIGHT/2.f - mm*2.f - ms*2.f});
	item_desc_shape.setFillColor(INV_ACCENT_COLOR);
	item_desc_shape.setOrigin(-(INV_WINDOW_WIDTH/2.f + mm*2.f + ms), -(ms));

	item_desc_obj = new TextBox("Description:", sf::Vector2f{0,0}, item_desc_shape.getSize().x - 20.f, BASE_FONT_NAME, INV_TEXT_COLOR, FontSize::TINY);
	item_desc_obj->setOrigin({-(INV_WINDOW_WIDTH/2.f + mm*2.f + ms + mm), -(ms + mm)});

	gui_objects.push_back(item_desc_obj);

	int iy = 0;
	for (auto i : items) {
		InvItemButton* ib1 = new InvItemButton(i, {0, 0}, INV_WINDOW_WIDTH/2.f - mm - ms);
		ib1->setOrigin({-ms, -(ms*(iy+1) + 2*(Item::items_texture_size + 16.f)*iy)});
		gui_objects.push_back(ib1);
		inv_buttons.push_back(ib1);
		++iy;
	}
}

void Inventory::Update()
{
	if (active) {
		window_shape.setPosition(window_tw.Tween());
		item_desc_shape.setPosition(window_tw.Tween());
		if (go_up_then_not_active && window_tw.getEnded()) {
			active = false;
			go_up_then_not_active = false;
		}

		for (auto o : gui_objects) {
			if (!window_tw.getEnded()) {
				o->setPos(window_tw.Tween());
			}
			o->Update();
		}
	}
}

void Inventory::Render(sf::RenderTarget & target)
{
	if (active) {
		target.setView(target.getDefaultView());
		target.draw(window_shape);
		target.draw(item_desc_shape);

		tooltip_render_target.clear(sf::Color::Transparent);

		for (auto o : gui_objects)
			o->Render(target, tooltip_render_target);

		tooltip_render_target.display();
		target.draw(tooltip_render_target_sprite);
	}
}

bool Inventory::HandleEvents(sf::Event const & event)
{
	bool ret = false;
	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == controls->get("Inventaire")) {
			setActive(false);
			ret = true;
		}
	}
	
	else if (event.type == sf::Event::MouseMoved) {
		sf::Vector2i mouse = {event.mouseMove.x, event.mouseMove.y};

		for (auto o : gui_objects) {
			if (o->isClicked())
				o->UpdateClickDrag(mouse);

			if (o->isMouseIn(mouse)) {
				if (!o->getHovered())
					if (o->onHoverIn(mouse)) ret = true;
				else 
					o->UpdateHoveredMousePos(mouse);
			}

			else if (o->getHovered())
				if (o->onHoverOut()) ret = true;
		}
	}

	else if (event.type == sf::Event::MouseButtonPressed) {
		sf::Vector2i mouse = {event.mouseMove.x, event.mouseMove.y};
		for (auto o : inv_buttons) o->setSelected(false);
		for (auto o : gui_objects) {
			if (o->getHovered())
				if (o->onClick(mouse)) return true;
		}
		for (auto o : inv_buttons) {
			if (o->getSelected()) {
				selected_item = o->getItem();
				ResetItemDescription();
			}
		}
	}
	else if (event.type == sf::Event::MouseButtonReleased) {
		for (auto o : gui_objects)
			if (o->isClicked())
				if (o->onClickRelease()) ret = true;
	}

	return ret;
}

void Inventory::ResetItemDescription()
{
	item_desc_obj->setTextString("Description: " + selected_item.desc);
}

void Inventory::AddItem(Item::any item)
{
	items.push_back(item);

	Init();
}

void Inventory::setActive(bool active)
{
	if (active) {
		window_tw.Reset(TweenType::QuintInOut, {WINDOW_X, WINDOW_STARTY}, {WINDOW_X, WINDOW_ENDY}, sf::seconds(0.2f));
		this->active = active;
	}
	else {
		go_up_then_not_active = true;
		window_tw.Reset(TweenType::QuintInOut, {WINDOW_X, WINDOW_ENDY}, {WINDOW_X, WINDOW_STARTY},  sf::seconds(0.2f));
	}
}
