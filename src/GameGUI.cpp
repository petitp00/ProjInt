#include "GameGUI.h"
#include "Game.h"
#include "GUIObjects.h"
#include "GameState.h"
#include "ResourceManager.h"

#include <iostream>
using namespace std;


static void eat(ButtonActionImpl* impl) {
	impl->game_state->getInventory()->EatItem(impl->item);
}

static void put_down(ButtonActionImpl* impl) {
	impl->game_state->getInventory()->PutDownItem(impl->item);
}

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

void Inventory::Init(ButtonActionImpl* button_action_impl)
{
	this->button_action_impl = button_action_impl;

	for (int i = 0; i != inv_buttons.size(); ++i) {
		delete inv_buttons[i];
	}
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

	ResetItemButtons();
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
		for (auto o : inv_buttons) {
			if (!window_tw.getEnded()) {
				o->setPos(window_tw.Tween());
			}
			o->Update();
		}
		for (auto o : actions_buttons) {
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

		for (auto o : inv_buttons)
			o->Render(target, tooltip_render_target);

		for (auto o : actions_buttons)
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
		for (auto o : inv_buttons) {
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
		for (auto o : actions_buttons) {
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

		InvItemButton* old_selected = nullptr;
		for (auto o : inv_buttons) {
			if (o->getSelected()) {
				old_selected = o;
				break;
			}
		}
		for (auto o : inv_buttons) o->setSelected(false);

		bool clicked = false;
		for (auto o : gui_objects) {
			if (o->getHovered()) {
				clicked = true;
				if (o->onClick(mouse)) { return true; }
			}
		}
		for (auto o : inv_buttons) {
			if (o->getHovered()) {
				clicked = true;
				if (o->onClick(mouse)) { return true; }
			}
		}
		for (auto o : actions_buttons) {
			if (o->getHovered()) {
				clicked = true;
				for (auto o : inv_buttons) o->setSelected(false);
				if (o->onClick(mouse)) { 
					ResetItemDescription(false);
					return true;
				}
			}
		}

		ResetItemDescription(false);

		for (auto o : inv_buttons) {
			if (o->getSelected()) {
				selected_item = o->getItem();
				ResetItemDescription();
			}
		}
	} else if (event.type == sf::Event::MouseButtonReleased) {
		for (auto o : gui_objects)
			if (o->isClicked())
				if (o->onClickRelease()) ret = true;
		for (auto o : actions_buttons)
			if (o->isClicked())
				if (o->onClickRelease()) ret = true;
	}

	return ret;
}

void Inventory::ResetItemButtons()
{
	for (int i = 0; i != inv_buttons.size(); ++i) {
		delete inv_buttons[i];
	}
	
	inv_buttons.clear();

	int iy = 0;
	for (auto i : items) {
		InvItemButton* ib1 = new InvItemButton(i, {0, 0}, INV_WINDOW_WIDTH/2.f - margin_middle - margin_sides);
		ib1->setOrigin({-margin_sides, -(margin_sides*(iy+1) + 2*(Item::items_texture_size + 16.f)*iy)});
		inv_buttons.push_back(ib1);
		++iy;
	}

	for (auto o : inv_buttons) {
		o->setPos(window_tw.Tween());
	}
}

void Inventory::ResetItemDescription(bool item_selected)
{
	if (item_selected) {
		item_desc_obj->setTextString("Description: " + selected_item.desc);

		for (int i = 0; i != actions_buttons.size(); ++i) {
			delete actions_buttons[i];
		}
		actions_buttons.clear();

		int i = 0;
		float width = INV_WINDOW_WIDTH/2.f - margin_middle*2.f - margin_sides*2.f;
		float height = 0;
		float ox = INV_WINDOW_WIDTH/2.f + margin_middle*2.f + margin_sides;
		float oy = margin_sides + margin_middle + item_desc_shape.getLocalBounds().height;

		auto b1 = new InvActionButton("Déposer", selected_item, {margin_sides + margin_middle + width, margin_sides}, width);
		b1->setOrigin({-(ox), -(oy)});
		b1->setPos(window_tw.Tween());
		height = b1->getSize().y;
		b1->setOnClickAction(new function<void(ButtonActionImpl*)>(put_down), button_action_impl);
		actions_buttons.push_back(b1);
		++i;

		if (selected_item.edible) {
			auto b = new InvActionButton("Manger", selected_item, {margin_sides + margin_middle + width, margin_sides}, width);
			b->setOrigin({-(ox), -(oy + (height + margin_middle) * i)});
			b->setPos(window_tw.Tween());
			b->setOnClickAction(new function<void(ButtonActionImpl*)>(eat), button_action_impl);
			actions_buttons.push_back(b);
			++i;
		}
	}
	else {
		item_desc_obj->setTextString("Description: ");
		for (int i = 0; i != actions_buttons.size(); ++i) {
			delete actions_buttons[i];
		}
		actions_buttons.clear();
	}
}

void Inventory::AddItem(Item::any item)
{
	items.push_back(item);

	ResetItemButtons();
}

void Inventory::RemoveItem(Item::any item)
{
	for (uint i = 0; i != items.size(); ++i) {
		if (items[i] == item) {
			items.erase(items.begin() + i);
			break;
		}
	}

	ResetItemButtons();
}

void Inventory::EatItem(Item::any item)
{
	RemoveItem(item);
}

void Inventory::PutDownItem(Item::any item)
{
	RemoveItem(item);
	ItemObject* i = make_item(item);
	button_action_impl->game_state->getWorld().StartPlaceItem(i);
	setActive(false);
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

InventoryButton::InventoryButton()
{
}

void InventoryButton::Init(Inventory* inventory)
{
	this->inventory = inventory;

	float scale = 3;
	size = {32 * scale, 32*scale};
	pos = {10, WINDOW_HEIGHT - 10 - size.y};
	sprite.setTexture(ResourceManager::getTexture("InventoryBagButton.png"));
	sprite.setTextureRect(sf::IntRect(0, 0, 32, 32));
	sprite.setScale(scale, scale);
	sprite.setPosition(pos);
}

void InventoryButton::Render(sf::RenderTarget & target)
{
	target.draw(sprite);
}

void InventoryButton::HandleEvent(sf::Event const & event)
{
	if (event.type == sf::Event::MouseMoved) {
		sf::Vector2f mouse = sf::Vector2f(float(event.mouseMove.x), float(event.mouseMove.y));

		if (mouse.x >= pos.x && mouse.x <= pos.x + size.x && mouse.y >= pos.y && mouse.y <= pos.y + size.y) {
			if (!open) {
				open = true;
				UpdateOpen();
			}
		}
		else if (open) {
			open = false;
			UpdateOpen();
		}
	}
	else if (event.type == sf::Event::MouseButtonPressed) {
		if (open && event.mouseButton.button == sf::Mouse::Left) {
			inventory->setActive(true);
		}
	}
}

void InventoryButton::UpdateOpen()
{
	if (open) {
		sprite.setTextureRect(sf::IntRect(32, 0, 32, 32));
	}
	else {
		sprite.setTextureRect(sf::IntRect(0, 0, 32, 32));
	}
}

