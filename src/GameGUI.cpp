#include "GameGUI.h"
#include "Globals.h"
#include "GUIObjects.h"
#include "GameState.h"
#include "ResourceManager.h"

#include <iostream>
using namespace std;

static void eat(ButtonActionImpl* impl) {
	auto item = Item::Manager::getAny(impl->item);
	if (Item::IsFood(Item::getItemTypeByName(item->name))) {
		auto food = Item::Manager::getFood(impl->item);
		if (food->junk_created != Item::none) {
			auto bp = Item::Manager::CreateItem(food->junk_created);
			impl->game_state->getInventory()->AddItem(bp);
		}
	}
	impl->game_state->getInventory()->EatItem(impl->item);
}

static void use_tool(ButtonActionImpl* impl) {
	auto tool = Item::Manager::getTool(impl->item);
	if (tool) {	
		auto tool_type = Item::getItemTypeByName(tool->name);

		if (tool_type != Item::ItemType::bowl) {
			tool->durability += 5;
			if (tool->durability == 100) {
				impl->game_state->getInventory()->DeleteItem(impl->item);
				if (impl->game_state->getEquippedTool() == impl->item) {
					impl->game_state->UnequipTool();
				}
			}
		}
		else {
			auto b = Item::Manager::getBowl(impl->item);
			b->UpdatePosInTextureMap();
		}

		impl->game_state->getEquippedToolObj()->UpdateDurability();
		impl->game_state->getInventory()->UpdateToolsDurability();
	}
}

static void repair_tool(ButtonActionImpl* impl) {
	auto tool = Item::Manager::getTool(impl->item);
	if (tool) {
		tool->durability = 0;
		impl->game_state->getEquippedToolObj()->UpdateDurability();
		impl->game_state->getInventory()->UpdateToolsDurability();
	}
}

static void equip_tool(ButtonActionImpl* impl) {
	auto tool = Item::Manager::getTool(impl->item);
	if (tool) {
		impl->game_state->EquipTool(impl->item);
	}
}

static void put_down(ButtonActionImpl* impl) {
	impl->game_state->getInventory()->PutDownItem(impl->item);
}

static void craft(ButtonActionImpl* impl) {
	
}

static void go_to_items_page(ButtonActionImpl* impl) {
	impl->game_state->getInventory()->GoToPage(PageType::Items);
}
static void go_to_tools_page(ButtonActionImpl* impl) {
	impl->game_state->getInventory()->GoToPage(PageType::Tools);
}
static void go_to_craft_page(ButtonActionImpl* impl) {
	impl->game_state->getInventory()->GoToPage(PageType::Craft);
}

static bool go_up_then_not_active = false;

static float INV_WINDOW_WIDTH = WINDOW_WIDTH / 4.f * 3;
static float INV_WINDOW_HEIGHT = WINDOW_HEIGHT / 4.f * 3;

static float WINDOW_X = (WINDOW_WIDTH - INV_WINDOW_WIDTH)/2.f;
static float WINDOW_STARTY = -INV_WINDOW_HEIGHT;
static float WINDOW_ENDY = (WINDOW_HEIGHT - INV_WINDOW_HEIGHT)/2.f;

static float margin_sides = 20.f;
static float margin_middle = 10.f;

template <typename T>
void UpdateObj(vector<T*>& vec, PosTweener& window_tw) {
	for (auto o : vec) {
		o->setPos(window_tw.Tween());
		o->Update();
	}
}

template <typename T>
bool MouseMoveO(vector<T*>& vec, const sf::Event& event) {
	vec2i mouse = {event.mouseMove.x, event.mouseMove.y};
	bool ret = false;

	for (auto o : vec) {
		if (o->isClicked())
			o->UpdateClickDrag(mouse);

		if (o->isMouseIn(mouse)) {
			if (!o->getHovered()) {
				if (o->onHoverIn(mouse)) { ret = true; }
			}
			else {
				o->UpdateHoveredMousePos(mouse);
			}
		}
		else if (o->getHovered())
			if (o->onHoverOut()) ret = true;
	}
	return ret;
}

// INVENTORY PAGES


InvPage::~InvPage()
{
	Clear();
}

void InvPage::Init()
{
	Clear();
	auto ms = margin_sides;
	auto mm = margin_middle;
	item_desc_shape.setSize({INV_WINDOW_WIDTH/2.f - mm*2.f - ms*2.f, INV_WINDOW_HEIGHT/2.f - mm*2.f - ms*2.f});
	item_desc_shape.setFillColor(INV_ACCENT_COLOR);
	item_desc_shape.setOrigin(-(INV_WINDOW_WIDTH/2.f + mm*2.f + ms), -(ms));
	item_desc_obj = new TextBox("Description:", vec2{0,0}, item_desc_shape.getSize().x - 20.f, BASE_FONT_NAME, INV_TEXT_COLOR, FontSize::TINY);
	item_desc_obj->setOrigin({-(INV_WINDOW_WIDTH/2.f + mm*2.f + ms + mm), -(ms + mm)});
	gui_objects.push_back(item_desc_obj);
}

void InvPage::Clear()
{
	for (int i = 0; i != gui_objects.size(); ++i) {
		delete gui_objects[i];
	}
	gui_objects.clear();

	for (int i = 0; i != actions_buttons.size(); ++i) {
		delete actions_buttons[i];
	}
	actions_buttons.clear();
}

ItemsPage::~ItemsPage()
{
	Clear();
}

void ItemsPage::Init()
{
	Clear();
	InvPage::Init();
}

void ItemsPage::Clear()
{
	InvPage::Clear();
	for (int i = 0; i != inv_buttons.size(); ++i) {
		delete inv_buttons[i];
	}
	inv_buttons.clear();
}

void ItemsPage::Update()
{
	item_desc_shape.setPosition(window_tw->Tween());
	UpdateObj<GUIObject>(gui_objects, *window_tw);
	UpdateObj<InvItemButton>(inv_buttons, *window_tw);
	UpdateObj<InvActionButton>(actions_buttons, *window_tw);
}

void ItemsPage::Render(sf::RenderTarget & target, sf::RenderTarget& tooltip_render_target)
{
	target.draw(item_desc_shape);
	for (auto o : gui_objects) o->Render(target, tooltip_render_target);
	for (auto o : inv_buttons) o->Render(target, tooltip_render_target);
	for (auto o : actions_buttons) o->Render(target, tooltip_render_target);
}

bool ItemsPage::HandleEvents(sf::Event const & event)
{
	bool ret = false;
	if (event.type == sf::Event::MouseMoved) {
		MouseMoveO<GUIObject>(gui_objects, event);
		MouseMoveO<InvItemButton>(inv_buttons, event);
		MouseMoveO<InvActionButton>(actions_buttons, event);
	}
	else if (event.type == sf::Event::MouseButtonPressed) {
		vec2i mouse = {event.mouseMove.x, event.mouseMove.y};
		int old_item = *selected_item;
		bool reselect = false;

		for (auto o : inv_buttons) o->setSelected(false); // unselect every item
		for (auto o : gui_objects)
			if (o->getHovered() && o->onClick(mouse)) return true;
		for (auto o : inv_buttons)
			if (o->getHovered() && o->onClick(mouse)) return true;
		for (auto o : actions_buttons) {
			if (o->getHovered()) {
				if (o->onClick(mouse)) { 
					ResetItemDescription(false);
					*selected_item = old_item;
					reselect = true;
					break;
				}
			}
		}

		if (!reselect) {
			ResetItemDescription(false);
		}
		else {
			bool found = false;
			for (auto o : inv_buttons) {
				if (o->getItem() == old_item) {
					o->setSelected(true);
					found = true;
					break;
				}
			}
			if (!found) return true; // the item is not in the inventory anymore so we return
		}

		for (auto o : inv_buttons) {
			if (o->getSelected()) {
				*selected_item = o->getItem();
				ResetItemDescription();
			}
		}
	}
	else if (event.type == sf::Event::MouseButtonReleased) {
		for (auto o : gui_objects)
			if (o->isClicked() && o->onClickRelease()) ret = true;
		for (auto o : actions_buttons)
			if (o->isClicked() && o->onClickRelease()) ret = true;
	}
	return ret;
}

void ItemsPage::ResetItemButtons(std::vector<int>& items)
{
	for (int i = 0; i != inv_buttons.size(); ++i) {
		delete inv_buttons[i];
	}
	
	inv_buttons.clear();

	int iy = 0;
	for (auto i : items) {
		auto item = Item::Manager::getAny(i);
		if (!Item::IsTool(Item::getItemTypeByName(item->name))) {
			InvItemButton* ib1 = new InvItemButton(i, {0, 0}, INV_WINDOW_WIDTH/2.f - margin_middle - margin_sides);
			ib1->setOrigin({-margin_sides, -(margin_sides*(iy+1) + (Item::items_texture_size + 20.f)*iy)});
			inv_buttons.push_back(ib1);
			++iy;
		}
	}

	for (auto o : inv_buttons) {
		o->setPos(window_tw->Tween());
	}
}

void ItemsPage::ResetItemDescription(bool item_selected)
{
	if (item_selected) {
		auto si = Item::Manager::getAny(*selected_item);
		auto sit = Item::getItemTypeByName(si->name);

		item_desc_obj->setTextString("Description: " + si->desc);

		for (int i = 0; i != actions_buttons.size(); ++i) {
			delete actions_buttons[i];
		}
		actions_buttons.clear();

		int i = 0;
		float width = INV_WINDOW_WIDTH/2.f - margin_middle*2.f - margin_sides*2.f;
		float height = 0;
		float ox = INV_WINDOW_WIDTH/2.f + margin_middle*2.f + margin_sides;
		float oy = margin_sides + margin_middle + item_desc_shape.getLocalBounds().height;

		auto b1 = new InvActionButton("Déposer", *selected_item, {margin_sides + margin_middle + width, margin_sides}, width);
		b1->setOrigin({-(ox), -(oy)});
		b1->setPos(window_tw->Tween());
		height = b1->getSize().y;
		b1->setOnClickAction(new function<void(ButtonActionImpl*)>(put_down), button_action_impl);
		actions_buttons.push_back(b1);
		++i;

		if (Item::IsFood(sit)) {
			auto b = new InvActionButton("Manger", *selected_item, {margin_sides + margin_middle + width, margin_sides}, width);
			b->setOrigin({-(ox), -(oy + (height + margin_middle) * i)});
			b->setPos(window_tw->Tween());
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

// 

ToolsPage::~ToolsPage()
{
	Clear();
}

void ToolsPage::Init()
{
	Clear();
	InvPage::Init();
}

void ToolsPage::Clear()
{
	for (int i = 0; i != inv_buttons.size(); ++i) {
		delete inv_buttons[i];
	}
	inv_buttons.clear();
}

void ToolsPage::Update()
{
	item_desc_shape.setPosition(window_tw->Tween());
	UpdateObj<GUIObject>(gui_objects, *window_tw);
	UpdateObj<InvToolButton>(inv_buttons, *window_tw);
	UpdateObj<InvActionButton>(actions_buttons, *window_tw);
}

void ToolsPage::Render(sf::RenderTarget & target, sf::RenderTarget& tooltip_render_target)
{
	target.draw(item_desc_shape);
	for (auto o : gui_objects) o->Render(target, tooltip_render_target);
	for (auto o : inv_buttons) o->Render(target, tooltip_render_target);
	for (auto o : actions_buttons) o->Render(target, tooltip_render_target);
}

bool ToolsPage::HandleEvents(sf::Event const & event)
{
	bool ret = false;
	if (event.type == sf::Event::MouseMoved) {
		MouseMoveO<GUIObject>(gui_objects, event);
		MouseMoveO<InvToolButton>(inv_buttons, event);
		MouseMoveO<InvActionButton>(actions_buttons, event);
	}
	else if (event.type == sf::Event::MouseButtonPressed) {
		auto old_selected = *selected_tool;
		bool reselect = false;
		vec2i mouse = {event.mouseMove.x, event.mouseMove.y};

		for (auto o : inv_buttons) o->setSelected(false); // unselect every item

		for (auto o : gui_objects)
			if (o->getHovered() && o->onClick(mouse)) return true;
		for (auto o : inv_buttons)
			if (o->getHovered() && o->onClick(mouse)) return true;
		for (auto o : actions_buttons) {
			if (o->getHovered()) {
				if (o->onClick(mouse)) { 
					ResetItemDescription(false);
					*selected_tool = old_selected;
					reselect = true;
					break;
				}
			}
		}
		if (!reselect) {
			ResetItemDescription(false); // clears the description box
		}
		else { // keep item selected
			bool found = false;
			for (auto o : inv_buttons) {
				if (o->getItem() == old_selected) {
					o->setSelected(true);
					found = true;
					break;
				}
			}
			if (!found) return true;
		}

		// for newly selected items
		for (auto o : inv_buttons) {
			if (o->getSelected()) {
				*selected_tool = o->getItem();
				ResetItemDescription();
			}
		}
	}
	else if (event.type == sf::Event::MouseButtonReleased) {
		for (auto o : gui_objects)
			if (o->isClicked() && o->onClickRelease()) ret = true;
		for (auto o : actions_buttons)
			if (o->isClicked() && o->onClickRelease()) ret = true;
	}
	return ret;
}

void ToolsPage::ResetItemButtons(std::vector<int>& items)
{
	for (int i = 0; i != inv_buttons.size(); ++i) {
		delete inv_buttons[i];
	}
	
	inv_buttons.clear();

	int iy = 0;
	for (auto i : items) {
		auto item = Item::Manager::getAny(i);
		if (Item::IsTool(Item::getItemTypeByName(item->name))) {
			InvToolButton* ib1 = new InvToolButton(i, {0, 0}, INV_WINDOW_WIDTH/2.f - margin_middle - margin_sides);
			ib1->setOrigin({-margin_sides, -(margin_sides*(iy+1) + (Item::items_texture_size + 20.f)*iy)});
			inv_buttons.push_back(ib1);
			++iy;
		}
	}

	for (auto o : inv_buttons) {
		o->setPos(window_tw->Tween());
	}
}

void ToolsPage::ResetItemDescription(bool item_selected)
{
	if (item_selected) {
		auto si = Item::Manager::getTool(*selected_tool);
		auto sit = Item::getItemTypeByName(si->name);

		item_desc_obj->setTextString("Description: " + si->desc);

		for (int i = 0; i != actions_buttons.size(); ++i) {
			delete actions_buttons[i];
		}
		actions_buttons.clear();

		int i = 0;
		float width = INV_WINDOW_WIDTH/2.f - margin_middle*2.f - margin_sides*2.f;
		float height = 0;
		float ox = INV_WINDOW_WIDTH/2.f + margin_middle*2.f + margin_sides;
		float oy = margin_sides + margin_middle + item_desc_shape.getLocalBounds().height;

		auto b2 = new InvActionButton("Équiper", *selected_tool, {margin_sides + margin_middle + width, margin_sides}, width);
		b2->setOrigin({-(ox), -(oy + (height + margin_middle) * i)});
		b2->setPos(window_tw->Tween());
		b2->setOnClickAction(new function<void(ButtonActionImpl*)>(equip_tool), button_action_impl);
		actions_buttons.push_back(b2);
		++i;

		height = b2->getSize().y;

		if (sit != Item::ItemType::bowl && si->durability > 0) {
			auto b = new InvActionButton("Réparer", *selected_tool, {margin_sides + margin_middle + width, margin_sides}, width);
			b->setOrigin({-(ox), -(oy + (height + margin_middle) * i)});
			b->setPos(window_tw->Tween());
			b->setOnClickAction(new function<void(ButtonActionImpl*)>(repair_tool), button_action_impl);
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

void ToolsPage::UpdateToolsDurability()
{
	for (auto o : inv_buttons) {
		o->UpdateDurability();
	}
}

// 

CraftPage::~CraftPage() { Clear(); }

float craft_item_desc_shape_height;

void CraftPage::Init()
{
	Clear();
	InvPage::Init();

	auto is = item_desc_shape.getSize();
	item_desc_shape.setSize(vec2(is.x, is.y*1.5f));
	craft_item_desc_shape_height = is.y*1.5f;

	recipe_box = new TextBox("Recette: ", vec2(0, 0), item_desc_shape.getSize().x - 20.f, BASE_FONT_NAME, INV_TEXT_COLOR, FontSize::TINY);
	auto ms = margin_sides;
	auto mm = margin_middle;
	recipe_box->setOrigin({-(INV_WINDOW_WIDTH/2.f + mm*2.f + ms + mm), -(ms + item_desc_shape.getSize().y/2.f)});
	gui_objects.push_back(recipe_box);
}

void CraftPage::Clear()
{
	for (int i = 0; i != inv_recipes.size(); ++i) {
		delete inv_recipes[i];
	}
	inv_recipes.clear();
}

void CraftPage::Update()
{
	item_desc_shape.setPosition(window_tw->Tween());
	UpdateObj<GUIObject>(gui_objects, *window_tw);
	UpdateObj<InvRecipeButton>(inv_recipes, *window_tw);
	UpdateObj<InvActionButton>(actions_buttons, *window_tw);
}

void CraftPage::Render(sf::RenderTarget & target, sf::RenderTarget& tooltip_render_target)
{
	target.draw(item_desc_shape);
	for (auto o : gui_objects) o->Render(target, tooltip_render_target);
	for (auto o : inv_recipes) o->Render(target, tooltip_render_target);
	for (auto o : actions_buttons) o->Render(target, tooltip_render_target);
}

bool CraftPage::HandleEvents(sf::Event const & event)
{
	bool ret = false;
	if (event.type == sf::Event::MouseMoved) {
		MouseMoveO<GUIObject>(gui_objects, event);
		MouseMoveO<InvRecipeButton>(inv_recipes, event);
		MouseMoveO<InvActionButton>(actions_buttons, event);
	}
	else if (event.type == sf::Event::MouseButtonPressed) {
		auto old_selected = *selected_recipe;
		bool reselect = false;
		vec2i mouse = {event.mouseMove.x, event.mouseMove.y};

		for (auto o : inv_recipes) o->setSelected(false); // unselect every item

		for (auto o : gui_objects)
			if (o->getHovered() && o->onClick(mouse)) return true;
		for (auto o : inv_recipes)
			if (o->getHovered() && o->onClick(mouse)) return true;
		for (auto o : actions_buttons) {
			if (o->getHovered()) {
				if (o->onClick(mouse)) {
					ResetItemDescription(false);
					*selected_recipe = old_selected;
					reselect = true;
					break;
				}
			}
		}
		if (!reselect) {
			ResetItemDescription(false); // clears the description box
		}
		else { // keep item selected
			bool found = false;
			for (auto o : inv_recipes) {
				if (o->getItemRecipe() == old_selected) {
					o->setSelected(true);
					found = true;
					break;
				}
			}
			if (!found) return true;
		}

		// for newly selected items
		for (auto o : inv_recipes) {
			if (o->getSelected()) {
				*selected_recipe = o->getItemRecipe();
				ResetItemDescription();
				break;
			}
		}
	}
	else if (event.type == sf::Event::MouseButtonReleased) {
		for (auto o : gui_objects)
			if (o->isClicked() && o->onClickRelease()) ret = true;
		for (auto o : actions_buttons)
			if (o->isClicked() && o->onClickRelease()) ret = true;
	}
	return ret;
}

void CraftPage::ResetRecipeButtons()
{
	for (int i = 0; i != inv_recipes.size(); ++i) {
		delete inv_recipes[i];
	}
	inv_recipes.clear();

	int iy = 0;
	for (auto i : Item::recipes) {
		auto b = new InvRecipeButton(i, {0,0}, INV_WINDOW_WIDTH/2.f - margin_middle - margin_sides);
		b->setOrigin({-margin_sides, -(margin_sides*(iy+1) + (Item::items_texture_size + 20.f) * iy)});
		inv_recipes.push_back(b);
		++iy;
	}
}

void CraftPage::ResetItemDescription(bool item_selected)
{
	if (item_selected) {
		auto sit = selected_recipe->first;
		auto tempid = Item::Manager::CreateItem(sit);
		auto temp = Item::Manager::getAny(tempid);
			
		item_desc_obj->setTextString("Description:\n" + temp->desc);

		auto ms = margin_sides;
		auto mm = margin_middle;

		float ry = max(ms + mm + mm + item_desc_obj->getSize().y, ms + item_desc_shape.getSize().y/2.f);

		recipe_box->setOrigin({-(INV_WINDOW_WIDTH/2.f + mm*2.f + ms + mm), -(ry)});

		bool can_craft; // set in Item::getRecipeString()
		recipe_box->setTextString("Recette:\n" + Item::getRecipeString(*selected_recipe, *items, &can_craft));

		for (auto b : inv_recipes) {
			if (b->getItemRecipe() == *selected_recipe) {
				b->setCraftable(can_craft);
			}
		}

		if (actions_buttons.size() == 0) {
			float width = INV_WINDOW_WIDTH/2.f - margin_middle*2.f - margin_sides*2.f;
			float ox = INV_WINDOW_WIDTH/2.f + margin_middle*2.f + margin_sides;
			float oy = margin_sides + margin_middle + item_desc_shape.getLocalBounds().height;
			auto b1 = new InvActionButton("Créer", sit, {margin_sides + margin_middle + width, margin_sides}, width);
			b1->setOrigin({-(ox), -(oy)});
			b1->setPos(window_tw->Tween());
			b1->setOnClickAction(new function<void(ButtonActionImpl*)>(craft), button_action_impl);
			actions_buttons.push_back(b1);
		}

		Item::Manager::DeleteItem(tempid);
	}
	else {
		// Clear description
		item_desc_obj->setTextString("Description: ");
		recipe_box->setTextString("Recette:");
		// Clear actions buttons
		for (int i = 0; i != actions_buttons.size(); ++i) { delete actions_buttons[i]; }
		actions_buttons.clear();
	}
}

void CraftPage::UpdateCanCraft()
{
	for (auto i : inv_recipes) {
		i->setCraftable(Item::getCanCraft(i->getItemRecipe(), *items));
	}
}

// INVENTORY //

Inventory::Inventory(Controls* controls) : controls(controls)
{
	active_page = &items_page;
	tooltip_render_target.create(WINDOW_WIDTH, WINDOW_HEIGHT);
	tooltip_render_target.setSmooth(true);
	tooltip_render_target_sprite.setTexture(tooltip_render_target.getTexture());
}

Inventory::~Inventory()
{
	for (int i = 0; i != page_buttons.size(); ++i) {
		delete page_buttons[i];
	}
}

void Inventory::Init(ButtonActionImpl* button_action_impl)
{
	this->button_action_impl = button_action_impl;

	items_page.button_action_impl = button_action_impl;
	items_page.window_tw = &window_tw;
	items_page.selected_item = &selected_item;
	items_page.Init();
	tools_page.button_action_impl = button_action_impl;
	tools_page.window_tw = &window_tw;
	tools_page.selected_tool = &selected_tool;
	tools_page.Init();
	craft_page.button_action_impl = button_action_impl;
	craft_page.window_tw = &window_tw;
	craft_page.selected_recipe = &selected_recipe;
	craft_page.items = &items;
	craft_page.Init();
	craft_page.UpdateCanCraft();

	for (int i = 0; i != page_buttons.size(); ++i) {
		delete page_buttons[i];
	}
	page_buttons.clear();

	window_shape.setSize({INV_WINDOW_WIDTH, INV_WINDOW_HEIGHT});
	window_shape.setFillColor(INV_WINDOW_COLOR);

	auto temp = new InvPageButton({0,0}, "", {0,0});
	delete temp;
	system("cls");
	cout << "The console has been cleared in Inventory::Init()" << endl;

	auto ms = margin_sides;
	auto mm = margin_middle;
	auto margin = InvPageButton::margin; // of InvPageButton
	auto ss = InvPageButton::side_size;

	auto page_butt1 = new InvPageButton({0,0}, Item::texture_map_file, {0, 0});
	page_butt1->setOrigin(vec2(-(-ss), -(ms)));
	page_butt1->setSelected(true);
	page_butt1->setOnClickAction(new function<void(ButtonActionImpl*)>(go_to_items_page), button_action_impl);
	page_butt1->setTooltip(new Tooltip("Items", sf::seconds(0.5f)));
	page_buttons.push_back(page_butt1);

	auto page_butt2 = new InvPageButton({0,0}, Item::texture_map_file, {6, 0});
	page_butt2->setOrigin(vec2(-(-ss), -(ms*2.f + ss)));
	page_butt2->setOnClickAction(new function<void(ButtonActionImpl*)>(go_to_tools_page), button_action_impl);
	page_butt2->setTooltip(new Tooltip("Outils", sf::seconds(0.5f)));
	page_buttons.push_back(page_butt2);

	auto page_butt3 = new InvPageButton({0,0}, Item::texture_map_file, {13, 0});
	page_butt3->setOrigin(vec2(-(-ss), -(ms*3.f + ss*2.f)));
	page_butt3->setOnClickAction(new function<void(ButtonActionImpl*)>(go_to_craft_page), button_action_impl);
	page_butt3->setTooltip(new Tooltip("Créer", sf::seconds(0.5f)));
	page_buttons.push_back(page_butt3);

	ResetItemButtons();
}


void Inventory::Update()
{
	if (active) {
		window_shape.setPosition(window_tw.Tween());
		if (go_up_then_not_active && window_tw.getEnded()) {
			active = false;
			go_up_then_not_active = false;
		}
		
		UpdateObj(page_buttons, window_tw);

		active_page->Update();
	}
}

void Inventory::Render(sf::RenderTarget & target)
{
	if (active) {
		target.setView(target.getDefaultView());
		target.draw(window_shape);

		tooltip_render_target.clear(sf::Color::Transparent);

		active_page->Render(target, tooltip_render_target);

		for (auto o : page_buttons)
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
		MouseMoveO<InvPageButton>(page_buttons, event);
	}
	else if (event.type == sf::Event::MouseButtonPressed) {
		vec2i mouse = {event.mouseMove.x, event.mouseMove.y};
		for (auto o : page_buttons) {
			if (o->getHovered()) {
				for (auto o2 : page_buttons) o2->setSelected(false);
				if (o->onClick(mouse)) return true;
			}
		}
	}
	else if (event.type == sf::Event::MouseButtonReleased) {
		for (auto o : page_buttons)
			if (o->isClicked() && o->onClickRelease()) ret = true;
	}

	if (active_page->HandleEvents(event)) ret = true;

	return ret;
}

void Inventory::ResetItemButtons()
{
	items_page.ResetItemButtons(items);
	tools_page.ResetItemButtons(items);
	craft_page.ResetRecipeButtons();
}

void Inventory::ResetItemDescription(bool item_selected)
{
	items_page.ResetItemDescription(item_selected);
	tools_page.ResetItemDescription(item_selected);
}

void Inventory::UpdateToolsDurability()
{
	tools_page.UpdateToolsDurability();
}

void Inventory::GoToPage(PageType type)
{
	if (type == PageType::Items)		active_page = &items_page;
	else if (type == PageType::Tools)	active_page = &tools_page;
	else if (type == PageType::Craft)	active_page = &craft_page;
}

bool Inventory::AddItem(int id)
{
	int tools_nb = 0;
	for (auto i : items) {
		auto item = Item::Manager::getAny(i);
		if (Item::IsTool(Item::getItemTypeByName(item->name))) {
			++tools_nb;
		}
	}

	auto ni = Item::Manager::getAny(id);
	if (Item::IsTool(Item::getItemTypeByName(ni->name))) {
		if (tools_nb < INV_MAX) {
			items.push_back(id);
		}
		else return false;
	}
	else if (items.size() - tools_nb < INV_MAX) {
		items.push_back(id);
	}
	else return false;

#ifndef EDITOR_MODE
	ResetItemButtons();
	craft_page.UpdateCanCraft();
#endif
	return true;
}

void Inventory::AddNewItem(Item::ItemType type)
{
	int item = Item::Manager::CreateItem(type);
	if (!AddItem(item)) Item::Manager::DeleteItem(item);
	
	craft_page.UpdateCanCraft();
}

void Inventory::RemoveItem(int item)
{
	for (uint i = 0; i != items.size(); ++i) {
		if (items[i] == item) {
			items.erase(items.begin() + i);
			break;
		}
	}

	ResetItemButtons();
	craft_page.UpdateCanCraft();
}

void Inventory::DeleteItem(int id)
{
	for (uint i = 0; i != items.size(); ++i) {
		if (items[i] == id) {
			items.erase(items.begin() + i);
			break;
		}
	}

	Item::Manager::DeleteItem(id);

	ResetItemButtons();
	craft_page.UpdateCanCraft();
	//ResetItemDescription();
}

void Inventory::EatItem(int item)
{
	RemoveItem(item);
}

void Inventory::PutDownItem(int item)
{
	RemoveItem(item);
	ItemObject* i = make_item(item);
	button_action_impl->game_state->getWorld().StartPlaceItem(i);
	setActive(false);
}

void Inventory::UseEquippedTool()
{
	auto tool = button_action_impl->game_state->getEquippedTool();
	if (tool != -1) {
		button_action_impl->item = tool;
		use_tool(button_action_impl);
	}
}

bool Inventory::IsMouseIn(vec2i mpos)
{
	auto p = window_tw.Tween();
	auto s = vec2(INV_WINDOW_WIDTH, INV_WINDOW_HEIGHT);
	return (mpos.x >= p.x && mpos.x <= p.x + s.x && mpos.y >= p.y && mpos.y <= p.y + s.y);
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

InventoryButton::InventoryButton() {}

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
		vec2 mouse = vec2(float(event.mouseMove.x), float(event.mouseMove.y));

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
	if (open) sprite.setTextureRect(sf::IntRect(32, 0, 32, 32));
	else sprite.setTextureRect(sf::IntRect(0, 0, 32, 32));
}

EquippedToolObj::EquippedToolObj() { }

void EquippedToolObj::Init(Inventory * inventory)
{
	this->inventory = inventory;
	float scale = 2;
	float ts = Item::items_texture_size * scale;
	auto item = Item::Manager::getTool(tool);

	size = {ts,ts};
	pos = {WINDOW_WIDTH - 10 - size.x, WINDOW_HEIGHT - 10 - size.y};
	sprite.setTexture(ResourceManager::getTexture(Item::texture_map_file));
	if (item) {
		sprite.setTextureRect(sf::IntRect((item->pos_in_texture_map)*int(ts/scale), {int(ts/scale), int(ts/scale)}));
	}
	sprite.setPosition(pos);
	sprite.setScale(scale, scale);

	shape.setSize({ts + 4, ts + 4});
	shape.setPosition(pos - vec2{2.f, 2.f});
	shape.setOutlineColor(INV_WINDOW_COLOR);
	shape.setFillColor(INV_ACCENT_COLOR);
	shape.setOutlineThickness(2);

	durab_bar.setPosition(pos + vec2(-2, shape.getSize().y - 4));
	UpdateDurability();

}

void EquippedToolObj::Render(sf::RenderTarget & target)
{
	if (tool != -1) {
		target.draw(shape);
		target.draw(sprite);
		if (tool_type != Item::ItemType::bowl) {
			target.draw(durab_bar);
		}
	}
}

void EquippedToolObj::HandleEvent(sf::Event const & event)
{
	if (event.type == sf::Event::MouseButtonPressed) {
		auto mp = vec2i(event.mouseButton.x, event.mouseButton.y);
		if (mp.x >= pos.x && mp.x <= pos.x + size.x) {
			if (mp.y >= pos.y && mp.y <= pos.y + size.y) {
				
			}
		}
	}
}

void EquippedToolObj::UpdateDurability()
{
	if (tool != -1) {
		auto t = Item::Manager::getTool(tool);
		float perc = max(0.f, min(100.f, (1 - t->durability / 100.f)));
		float bw = shape.getSize().x * perc;

		if (t->durability >= 95)
			shape.setOutlineColor(sf::Color::Red);
		else shape.setOutlineColor(INV_WINDOW_COLOR);

		durab_bar.setSize(vec2(bw, 2));
		durab_bar.setFillColor(LerpColor(sf::Color::Red, sf::Color::Green, perc));

		if (Item::getItemTypeByName(t->name) == Item::ItemType::bowl) {
			float scale = 2;
			float ts = Item::items_texture_size * scale;
			sprite.setTextureRect(sf::IntRect((t->pos_in_texture_map)*int(ts/scale), {int(ts/scale), int(ts/scale)}));
		}
	}
}

void EquippedToolObj::setTool(int id)
{
	tool = id;
	if (id != -1) {
		auto i = Item::Manager::getAny(id);
		tool_type = Item::getItemTypeByName(i->name);
	}
	Init(inventory);
}
