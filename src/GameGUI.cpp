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
				impl->game_state->getInventory()->UnequipTool();
				impl->game_state->getInventory()->DeleteItem(impl->item);
				if (impl->game_state->getEquippedTool() == impl->item) {
					impl->game_state->UnequipTool();
					tool = nullptr;
				}
			}
		}
		else {
			auto b = Item::Manager::getBowl(impl->item);
			b->UpdatePosInTextureMap();
		}

		impl->game_state->getInventory()->Refresh();
		impl->game_state->getEquippedToolObj()->UpdateDurability();

		//impl->game_state->getInventory()->UpdateToolsDurability();
		//impl->game_state->getInventory()->ResetItemDescription();
	}
}

static void repair_tool(ButtonActionImpl* impl) {
	auto tool = Item::Manager::getTool(impl->item);
	if (tool) {
		auto recipe = Item::getToolRepairRecipe(Item::getItemTypeByName(tool->name));
		auto inv = impl->game_state->getInventory();
		if (Item::getCanCraft(recipe, inv->getItemsId())) {
			for (auto i : recipe.second) {
				inv->DeleteItemWithType(i.first, i.second);
			}
			tool->durability = 0;
			impl->game_state->getEquippedToolObj()->UpdateDurability();
			impl->game_state->getInventory()->Refresh();
		}
	}
}

static void equip_tool(ButtonActionImpl* impl) {
	auto tool = Item::Manager::getTool(impl->item);
	if (tool) {
		impl->game_state->EquipTool(impl->item);
	}
}

static void unequip_tool(ButtonActionImpl* impl) {
	impl->game_state->UnequipTool();
}

static void put_down(ButtonActionImpl* impl) {
	impl->game_state->getInventory()->PutDownItem(impl->item);
}

static void plant_seed(ButtonActionImpl* impl) {
	impl->game_state->getInventory()->PlantSeed(impl->item);
}

static void craft(ButtonActionImpl* impl) {
	auto recipe = impl->recipe;
	auto inv = impl->game_state->getInventory();
	if (Item::getCanCraft(recipe, inv->getItemsId())) {
		for (auto i : recipe.second) {
			inv->DeleteItemWithType(i.first, i.second);
		}

		inv->AddNewItem(recipe.first);
	}
	else {
		cout << "Can not craft item: " << Item::getItemName(recipe.first) << endl;
	}
}

static void craft_previous_page(ButtonActionImpl* impl) {
	impl->game_state->getInventory()->RecipePrevPage();
}

static void craft_next_page(ButtonActionImpl* impl) {
	impl->game_state->getInventory()->RecipeNextPage();
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


		/*
			when mouse is clicked
			we unselect every item and set selected_item to -1
			if the mouse clicked on an other item, we select it
			if the mouse clicked on an action button, we try to reselect the previously selected item
			if the item is not in the inventory anymore, we return from the function
			if it is found, we update the item description
		*/
	
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
		*selected_item = -1;
		bool reselect = false; // if we need to reselect the old item (we clicked on a button)

		for (auto o : inv_buttons) o->setSelected(false); // unselect every item

		for (auto o : inv_buttons) {
			if (o->getHovered() && o->onClick(mouse)) {
				*selected_item = o->getItem();
				ResetItemDescription();
				return true; // we selected another item so we do not need to execute the rest of the function
			}
		}

		for (auto o : actions_buttons) {
			if (o->getHovered()) {
				if (o->onClick(mouse)) { 
					ResetItemDescription(); // in case the action deleted the item from the inventory
					*selected_item = old_item;
					reselect = true;
					ret = true; // important
					break;
				}
			}
		}

		if (reselect) {
			for (auto o : inv_buttons) {
				if (o->getItem() == old_item) {
					o->setSelected(true);
					*selected_item = o->getItem();
					ResetItemDescription();
					return true;
				}
			}
			*selected_item = -1; // the item was not found, so we know it has been removed
		}
		else { // if we clicked away
			ResetItemDescription();
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

void ItemsPage::ResetItemDescription()
{
	if (selected_item && *selected_item != -1) {
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

		using butt_action_t = function<void(ButtonActionImpl*)>;
		auto add_action_button = [&] (const string& text, action_t func){
			auto b = new InvActionButton(text, *selected_item, {margin_sides + margin_middle + width, margin_sides}, width);
			b->setOrigin({-(ox), -(oy + (height + margin_middle) * i)});
			b->setPos(window_tw->Tween());
			b->setOnClickAction(func, button_action_impl);
			actions_buttons.push_back(b);
			if (height == 0) {
				height = b->getSize().y;
			}
			++i;
		};

		add_action_button("Déposer", new butt_action_t(put_down));

		if (Item::IsFood(sit)) {
			add_action_button("Manger", new butt_action_t(eat));
		}

		if (Item::IsSeed(sit)) {
			add_action_button("Planter", new butt_action_t(plant_seed));
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
		vec2i mouse = {event.mouseMove.x, event.mouseMove.y};

		auto old_selected = *selected_tool;
		*selected_tool = -1;
		bool reselect = false;

		for (auto o : inv_buttons) o->setSelected(false); // unselect every item

		for (auto o : inv_buttons) {
			if (o->getHovered() && o->onClick(mouse)) {
				*selected_tool = o->getItem();
				ResetItemDescription();
				return true;
			}
		}
		for (auto o : actions_buttons) {
			if (o->getHovered()) {
				if (o->onClick(mouse)) { 
					ResetItemDescription();
					*selected_tool = old_selected;
					reselect = true;
					ret = true;
					break;
				}
			}
		}
		if (reselect) {
			for (auto o : inv_buttons) {
				if (o->getItem() == old_selected) {
					o->setSelected(true);
					*selected_tool = old_selected;
					ResetItemDescription();
					return true;
				}
			}
			*selected_tool = -1;
		}
		else {
			ResetItemDescription();
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

void ToolsPage::ResetItemDescription()
{
	if (*selected_tool != -1) {
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

		InvActionButton* b2;

		if (*selected_tool != button_action_impl->game_state->getEquippedTool()) {
			b2 = new InvActionButton("Équiper", *selected_tool, { margin_sides + margin_middle + width, margin_sides }, width);
			b2->setOnClickAction(new function<void(ButtonActionImpl*)>(equip_tool), button_action_impl);
		}
		else {
			b2 = new InvActionButton("Déséquiper", *selected_tool, { margin_sides + margin_middle + width, margin_sides }, width);
			b2->setOnClickAction(new function<void(ButtonActionImpl*)>(unequip_tool), button_action_impl);
		}
		b2->setOrigin({ -(ox), -(oy + (height + margin_middle) * i) });
		b2->setPos(window_tw->Tween());
		actions_buttons.push_back(b2);
		++i;
		height = b2->getSize().y;

		if (sit != Item::ItemType::bowl) {
			auto recipe = Item::getToolRepairRecipe(sit);
			bool can_repair = false;
			string repair_str = Item::getRecipeString(recipe, button_action_impl->game_state->getInventory()->getItemsId(), &can_repair);

			if (si->durability > 0 && can_repair) {
				auto b = new InvActionButton("Réparer", *selected_tool, { margin_sides + margin_middle + width, margin_sides }, width);
				b->setOrigin({ -(ox), -(oy + (height + margin_middle) * i) });
				b->setPos(window_tw->Tween());
				b->setOnClickAction(new function<void(ButtonActionImpl*)>(repair_tool), button_action_impl);
				actions_buttons.push_back(b);
				++i;

			}
			item_desc_obj->setTextString("Description: " + si->desc + '\n' + '\n' + "Réparer:" + '\n' + repair_str);
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
float recipe_button_width = INV_WINDOW_WIDTH / 2.f - margin_middle - margin_sides;
void CraftPage::Init()
{
	Clear();
	InvPage::Init();

	int ri = Item::recipes.size();
	float result = float(ri) / float(max_recipes_per_page);
	max_page = int(result);
	if (max_page < result) ++max_page;

	for (int i = 0; i != max_page; ++i) {
		inv_recipes.push_back(vector<InvRecipeButton*>());
	}

	auto is = item_desc_shape.getSize();
	item_desc_shape.setSize(vec2(is.x, is.y*1.5f));
	craft_item_desc_shape_height = is.y*1.5f;

	recipe_box = new TextBox("Recette: ", vec2(0, 0), item_desc_shape.getSize().x - 20.f,
		BASE_FONT_NAME, INV_TEXT_COLOR, FontSize::TINY);
	auto ms = margin_sides;
	auto mm = margin_middle;
	recipe_box->setOrigin({-(INV_WINDOW_WIDTH/2.f + mm*2.f + ms + mm), -(ms + item_desc_shape.getSize().y/2.f)});
	gui_objects.push_back(recipe_box);

	page_button1 = new SquareButton("<", vec2(0, 0), 40, FontSize::TINY,
		INV_TEXT_COLOR, INV_ACCENT_COLOR, INV_ACCENT_COLOR2);
	page_button2 = new SquareButton(">", vec2(0, 0), 40, FontSize::TINY,
		INV_TEXT_COLOR, INV_ACCENT_COLOR, INV_ACCENT_COLOR2);
	page_button1->setOnClickAction(new function<void(ButtonActionImpl*)>(craft_previous_page), button_action_impl);
	page_button2->setOnClickAction(new function<void(ButtonActionImpl*)>(craft_next_page), button_action_impl);

	auto sz = page_button1->getSize();
	page_text = new TextBox(to_string(page + 1) + " sur " + to_string(max_page),
		vec2(0,0), 85, BASE_FONT_NAME, INV_TEXT_COLOR, FontSize::TINY);
}

void CraftPage::Clear()
{
	for (int i = 0; i != inv_recipes.size(); ++i) {
		for (int i2 = 0; i2 != inv_recipes[i].size(); ++i2) {
			delete inv_recipes[i][i2];
		}
	}
	inv_recipes.clear();
}

void CraftPage::Update()
{
	item_desc_shape.setPosition(window_tw->Tween());
	page_button1->setPos(vec2(-item_desc_shape.getOrigin().x, INV_WINDOW_HEIGHT - margin_sides - 56) + window_tw->Tween());
	page_button2->setPos(vec2(INV_WINDOW_WIDTH - margin_sides - 40, INV_WINDOW_HEIGHT - margin_sides - 56) + window_tw->Tween());
	page_text->setPos(vec2(-85/2.f + INV_WINDOW_WIDTH/4.f * 3.f, INV_WINDOW_HEIGHT - margin_sides - 52) + window_tw->Tween());
	UpdateObj<GUIObject>(gui_objects, *window_tw);
	UpdateObj<InvRecipeButton>(inv_recipes[page], *window_tw);
	UpdateObj<InvActionButton>(actions_buttons, *window_tw);
}

void CraftPage::Render(sf::RenderTarget & target, sf::RenderTarget& tooltip_render_target)
{
	target.draw(item_desc_shape);

	page_button1->Render(target, tooltip_render_target);
	page_button2->Render(target, tooltip_render_target);
	page_text->Render(target, tooltip_render_target);

	for (auto o : gui_objects) o->Render(target, tooltip_render_target);
	for (auto o : inv_recipes[page]) o->Render(target, tooltip_render_target);
	for (auto o : actions_buttons) o->Render(target, tooltip_render_target);
}

bool CraftPage::HandleEvents(sf::Event const & event)
{
	bool ret = false;
	if (event.type == sf::Event::MouseMoved) {
		MouseMoveO<GUIObject>(gui_objects, event);
		MouseMoveO<InvRecipeButton>(inv_recipes[page], event);
		MouseMoveO<InvActionButton>(actions_buttons, event);

		vec2i mouse = vec2i(event.mouseMove.x, event.mouseMove.y);
		if (page_button1->isMouseIn(mouse)) {
			if (!page_button1->getHovered()) { page_button1->onHoverIn(); }
		}
		else if (page_button1->getHovered()) { page_button1->onHoverOut(); }

		if (page_button2->isMouseIn(mouse)) {
			if (!page_button2->getHovered()) { page_button2->onHoverIn(); }
		}
		else if (page_button2->getHovered()) { page_button2->onHoverOut(); }
	}
	else if (event.type == sf::Event::MouseButtonPressed) {
		vec2i mouse = {event.mouseMove.x, event.mouseMove.y};

		auto old_selected = *selected_recipe;
		*selected_recipe = Item::no_recipe;
		bool reselect = false;

		if (page_button1->getHovered()) page_button1->onClick(mouse);
		if (page_button2->getHovered()) page_button2->onClick(mouse);

		for (auto o : inv_recipes[page]) o->setSelected(false); // unselect every item

		for (auto o : inv_recipes[page]) {
			if (o->getHovered() && o->onClick(mouse)) {
				*selected_recipe = o->getItemRecipe();
				ResetItemDescription();
				return true;
			}
		}

		for (auto o : actions_buttons) {
			if (o->getHovered()) {
				if (o->onClick(mouse)) {
					ResetItemDescription(); 
					*selected_recipe = old_selected;
					reselect = true;
					ret = true;
					break;
				}
			}
		}
		if (reselect) {
			for (auto o : inv_recipes[page]) {
				if (o->getItemRecipe() == old_selected) {
					o->setSelected(true);
					*selected_recipe = old_selected;
					ResetItemDescription(); 
					return true;
				}
			}
			*selected_recipe = Item::no_recipe;
		}
		else {
			ResetItemDescription();
		}
	}
	else if (event.type == sf::Event::MouseButtonReleased) {
		if (page_button1->isClicked()) { page_button1->onClickRelease(); }
		if (page_button2->isClicked()) { page_button2->onClickRelease(); }

		for (auto o : gui_objects)
			if (o->isClicked() && o->onClickRelease()) ret = true;
		for (auto o : actions_buttons)
			if (o->isClicked() && o->onClickRelease()) ret = true;
	}
	else if (event.type == sf::Event::MouseWheelScrolled) {
		float scroll_speed = 20;
		auto delta = event.mouseWheelScroll.delta;
	}
	return ret;
}

void CraftPage::ResetRecipeButtons()
{
	for (int i = 0; i != inv_recipes.size(); ++i) {
		for (int i2 = 0; i2 != inv_recipes[i].size(); ++i2) {
			delete inv_recipes[i][i2];
		}
		inv_recipes[i].clear();
	}

	int iy = 0;
	int add_to_page = 0;
	for (auto i : Item::recipes) {
		auto b = new InvRecipeButton(i, {0,0}, INV_WINDOW_WIDTH/2.f - margin_middle - margin_sides, button_action_impl);
		b->setOrigin({-margin_sides, -(margin_sides*(iy+1) + (Item::items_texture_size + 20.f) * iy)});
		inv_recipes[add_to_page].push_back(b);
		++iy;
		if (iy == max_recipes_per_page) {
			iy = 0;
			++add_to_page;
		}
	}
}

void CraftPage::ResetItemDescription()
{
	if (selected_recipe && *selected_recipe != Item::no_recipe) {
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

		for (auto b : inv_recipes[page]) {
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
	for (auto p : inv_recipes) {
		for (auto i : p) {
			i->setCraftable(Item::getCanCraft(i->getItemRecipe(), *items));
		}
	}
}

void CraftPage::PrevPage()
{
	if (page > 0) {
		--page;
	}
	UpdatePageText();
}

void CraftPage::NextPage()
{
	if (page != max_page - 1) {
		++page;
	}
	UpdatePageText();
}

void CraftPage::UpdatePageText()
{
	page_text->setTextString(to_string(page + 1) + " sur " + to_string(max_page));
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
	
	selected_recipe = Item::no_recipe;

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

	//ResetItemButtons();
	Refresh();
}

void Inventory::Clear()
{
	items.clear();
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

void Inventory::Refresh()
{
	items_page.ResetItemButtons(items);
	tools_page.ResetItemButtons(items);
	craft_page.ResetRecipeButtons();

	items_page.ResetItemDescription();
	tools_page.ResetItemDescription();
	craft_page.ResetItemDescription();

	tools_page.UpdateToolsDurability();
	
	craft_page.UpdateCanCraft();
}

void Inventory::GoToPage(PageType type)
{
	if (type == PageType::Items)		active_page = &items_page;
	else if (type == PageType::Tools)	active_page = &tools_page;
	else if (type == PageType::Craft)	active_page = &craft_page;
}

bool Inventory::AddItem(int id)
{
	bool full = false;

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
		else full = true;
	}
	else if (items.size() - tools_nb < INV_MAX) {
		items.push_back(id);
	}
	else full = true;

	if (full) {
		button_action_impl->game_state->getWorld().DropItemFromInventory(id);
	}

#ifndef EDITOR_MODE
	Refresh();
#endif

	return !full;
}

void Inventory::AddNewItem(Item::ItemType type)
{
	int item = Item::Manager::CreateItem(type);
	AddItem(item);
	
	Refresh();
}

void Inventory::RemoveItem(int item)
{
	for (uint i = 0; i != items.size(); ++i) {
		if (items[i] == item) {
			items.erase(items.begin() + i);
			break;
		}
	}

	Refresh();
}

void Inventory::DeleteItemWithType(Item::ItemType type, size_t n)
{
	int deleted = 0;
	for (auto i = items.begin(); i != items.end();) {
		auto itype = Item::Manager::getItemType(*i);
		if (itype == type) {
			int id = *i;
			i = items.erase(i);
			DeleteItem(id); // the for-loop at the start won't find the item, but the rest will work

			++deleted;
			if (deleted == n) return;
			continue;
		}
		++i;
	}
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
	if (selected_item == id) selected_item = -1;
	if (selected_tool == id) selected_tool = -1;

	Refresh();
	craft_page.UpdateCanCraft();
}

void Inventory::EatItem(int item)
{
	RemoveItem(item);
	Refresh();
}

void Inventory::PutDownItem(int item)
{
	RemoveItem(item);
	ItemObject* i = make_item(item);
	button_action_impl->game_state->getWorld().StartPlaceItem(i);
	setActive(false);
	Refresh();
}

void Inventory::PlantSeed(int id)
{
	RemoveItem(id);
	ItemObject* i = make_item(id);
	button_action_impl->game_state->getWorld().StartPlantItem(i);
	setActive(false);
	Refresh();
}

void Inventory::UnequipTool()
{
	selected_tool = -1;
	Refresh();
}

void Inventory::UseEquippedTool()
{
	auto tool = button_action_impl->game_state->getEquippedTool();
	if (tool != -1) {
		button_action_impl->item = tool;
		use_tool(button_action_impl);
		if (tool != -1) {
			Refresh();
		}
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

void EquippedToolObj::Init(Inventory * inventory, GameState* game_state)
{
	this->inventory = inventory;
	this->game_state = game_state;
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
				if (game_state) {
					game_state->UnequipTool();
				}
				else {
					cerr << "WATCH OUT: game_state is nullptr in EquippedToolObj. (line: " << __LINE__ << ")" << endl;
				}
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
	Init(inventory, game_state);
}

sf::Time change_time = sf::seconds(0.25f);

GUIHoverInfo::GUIHoverInfo()
{
	size = vec2(0, 0);
	origin = vec2(float(WINDOW_WIDTH), 20);
	pos = origin;

	margin = 12.f;

	text_obj.setFont(ResourceManager::getFont(BASE_FONT_NAME));
	text_obj.setCharacterSize(FontSize::TINY);

	rect.setFillColor(INV_WINDOW_COLOR);

	rect_alpha_tw.Reset(TweenType::Linear, 1, 0, sf::seconds(0.1f));
	text_alpha_tw.Reset(TweenType::Linear, 0, 1, sf::seconds(0.1f));
}

void GUIHoverInfo::Update()
{
	auto tcol = INV_TEXT_COLOR;
	tcol.a = 0;
	text_obj.setFillColor(LerpColor(tcol, INV_TEXT_COLOR, text_alpha_tw.Tween()));

	auto rcol = INV_WINDOW_COLOR;
	rcol.a = 0;
	rect.setFillColor(LerpColor(rcol, INV_WINDOW_COLOR, rect_alpha_tw.Tween()));
}

void GUIHoverInfo::Render(sf::RenderTarget & target)
{
	target.draw(rect);
	target.draw(text_obj);
}

void GUIHoverInfo::setString(const std::string & text)
{
	if (text == "") {
		this->text = text;
		rect_alpha_tw.Reset(TweenType::QuartOut, 1, 0, change_time);
		text_alpha_tw.Reset(TweenType::QuartOut, 1, 0, change_time);
	}
	else if (text != this->text) {
		if (*(text.end() - 1) != '\n') {
			this->text = text + '\n';
		}
		else {
			this->text = text;
		}
		rect_alpha_tw.Reset(TweenType::QuartInOut, 0, 1, change_time);
		text_alpha_tw.Reset(TweenType::QuartIn, 0, 1, change_time);

		text_obj.setString(this->text);

		size = vec2(text_obj.getGlobalBounds().width, text_obj.getGlobalBounds().height);
		size += vec2(margin * 2, margin * 2 - 10);
		pos = origin - vec2(size.x, 0);
		rect.setPosition(pos);
		rect.setSize(size);
		text_obj.setPosition(pos + vec2(margin, margin));
	}
}

GUIActionInfo::GUIActionInfo()
{
	size = vec2(0, 0);
	origin = vec2(WINDOW_WIDTH/2.f, WINDOW_HEIGHT);
	pos = origin;

	margin = 12.f;

	text_obj.setFont(ResourceManager::getFont(BASE_FONT_NAME));
	text_obj.setCharacterSize(FontSize::TINY);

	rect.setFillColor(INV_WINDOW_COLOR);

	rect_alpha_tw.Reset(TweenType::Linear, 1, 0, sf::seconds(0.1f));
	text_alpha_tw.Reset(TweenType::Linear, 0, 1, sf::seconds(0.1f));
} 

void GUIActionInfo::Update()
{
	auto col = INV_TEXT_COLOR;
	col.a = 0;
	text_obj.setFillColor(LerpColor(col, INV_TEXT_COLOR, text_alpha_tw.Tween()));

	col = INV_WINDOW_COLOR;
	col.a = 0;
	rect.setFillColor(LerpColor(col, INV_WINDOW_COLOR, rect_alpha_tw.Tween()));
}

void GUIActionInfo::Render(sf::RenderTarget & target)
{
	target.draw(rect);
	target.draw(text_obj);
}

void GUIActionInfo::setActionInfo(ActionInfo info)
{
	switch (info)
	{
	case ActionInfo::none:
		setString("");
		break;
	case ActionInfo::plant:
		setString("Clic gauche pour planter\nClic droit pour annuler");
		break;
	case ActionInfo::place:
		setString("Clic droit pour déposer");
		break;
	case ActionInfo::fishing:
		setString("Clic droit pour sortir l'appât de l'eau");
		break;
	case ActionInfo::collect:
		setString("Clic gauche pour récolter");
		break;
	case ActionInfo::use_tool:
		setString("Clic droit pour utiliser outil");
		break;
	case ActionInfo::collect_or_use_tool:
		setString("Clic gauche pour récolter\nClic droit pour utiliser outil");
		break;
	default:
		break;
	}
}

void GUIActionInfo::setString(const std::string & text)
{
	if (text == "") {
		this->text = text;
		rect_alpha_tw.Reset(TweenType::QuartOut, 1, 0, change_time);
		text_alpha_tw.Reset(TweenType::QuartOut, 1, 0, change_time);
	}
	else if (text != this->text) {
		this->text = text;
		if (*(text.end() - 1) != '\n') {
			this->text = text + '\n';
		}
		rect_alpha_tw.Reset(TweenType::QuartInOut, 0, 1, change_time);
		text_alpha_tw.Reset(TweenType::QuartIn, 0, 1, change_time);

		text_obj.setString(this->text);

		size = vec2(text_obj.getGlobalBounds().width, text_obj.getGlobalBounds().height);
		size += vec2(margin * 2, margin * 2 - 10);
		pos = origin - vec2(size.x/2.f, size.y);
		rect.setPosition(pos);
		rect.setSize(size);
		text_obj.setPosition(pos + vec2(margin, margin));
	}
}
