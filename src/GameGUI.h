#pragma once

#include "Tweener.h"
#include "Items.h"
#include "Globals.h"
#include <SFML/Graphics.hpp>

class GameState;
class GUIObject;
class SquareButton;
class Scrollbar;
class TextBox;
class TextButton;
class InvItemButton;
class InvActionButton;
class InvPageButton;
class InvToolButton;
class InvRecipeButton;
class ButtonActionImpl;
struct Controls;

static sf::Color INV_WINDOW_COLOR(115, 106, 91, 200);
static sf::Color INV_ACCENT_COLOR(144, 131, 114, 210);
static sf::Color INV_ACCENT_COLOR2(135, 118, 96, 210);
static sf::Color INV_ACCENT_COLOR3(169, 145, 98, 210);
static sf::Color INV_TEXT_COLOR(47, 50, 55);

enum class PageType
{
	Items,
	Tools,
	Craft
};

struct InvPage
{
	InvPage()=default;
	virtual ~InvPage();

	virtual void Init();
	virtual void Clear();
	virtual void Update()=0;
	virtual void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target)=0;
	virtual bool HandleEvents(sf::Event const& event)=0;


	// with kept vars for position
	// void AddActionButton(PageType ptype, string name, action_t func, Item::ItemType itype)


	ButtonActionImpl* button_action_impl = nullptr;
	PosTweener* window_tw = nullptr;
	sf::RectangleShape item_desc_shape;
	TextBox* item_desc_obj;
	std::vector<GUIObject*> gui_objects;
	std::vector<InvActionButton*> actions_buttons;
};

struct ItemsPage : public InvPage
{
	~ItemsPage() override;
	void Init() override;
	void Clear() override;
	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target) override;
	bool HandleEvents(sf::Event const& event) override;
	void ResetItemButtons(std::vector<int>& items);
	void ResetItemDescription();

	int* selected_item;
	std::vector<InvItemButton*> inv_buttons;
};

struct ToolsPage : public InvPage
{
	~ToolsPage() override;
	void Init() override;
	void Clear() override;
	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target) override;
	bool HandleEvents(sf::Event const& event) override;
	void ResetItemButtons(std::vector<int>& items);
	void ResetItemDescription();
	void UpdateToolsDurability();

	int* selected_tool;
	std::vector<InvToolButton*> inv_buttons;
};

struct CraftPage : public InvPage
{
	CraftPage()=default;
	~CraftPage() override;
	void Init() override;
	void Clear() override;
	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target) override;
	bool HandleEvents(sf::Event const& event) override;
	void ResetRecipeButtons();
	void ResetItemDescription();
	void UpdateCanCraft();

	void PrevPage();
	void NextPage();
	void UpdatePageText();

	int page = 0;
	int max_page = 1;
	int max_recipes_per_page = 7;
	TextBox* page_text;

	Item::Recipe* selected_recipe;
	std::vector<int>* items;
	std::vector<std::vector<InvRecipeButton*>> inv_recipes;
	TextBox* recipe_box;
	SquareButton *page_button1, *page_button2;
};

class Inventory
{
public:
	Inventory(Controls* controls);
	~Inventory();
	void Init(ButtonActionImpl* button_action_impl);

	void Update();
	void Render(sf::RenderTarget& target);
	bool HandleEvents(sf::Event const& event);
	void Refresh();

	void GoToPage(PageType type);
	void RecipePrevPage() { craft_page.PrevPage(); }
	void RecipeNextPage() { craft_page.NextPage(); }

	// Items
	bool AddItem(int id);
	void AddNewItem(Item::ItemType type);
	void RemoveItem(int id);
	void DeleteItemWithType(Item::ItemType type, size_t n);
	void DeleteItem(int id);
	void EatItem(int id);
	void PutDownItem(int id);
	void PlantSeed(int id);
	void UnequipTool();
	void UseEquippedTool();

	// Getters
	bool getActive() { return active; }
	bool IsMouseIn(vec2i mpos);
	std::vector<int>& getItemsId() { return items; }

	// Setters
	void setActive(bool active);

private:
	Controls* controls;
	ButtonActionImpl* button_action_impl;
	bool active = false;
	std::vector<int> items;
	int selected_item = -1;
	int selected_tool = -1;
	Item::Recipe selected_recipe;

	sf::RenderTexture tooltip_render_target; // on top
	sf::Sprite tooltip_render_target_sprite;
	sf::RectangleShape window_shape;
	PosTweener window_tw;

	ItemsPage items_page;
	ToolsPage tools_page;
	CraftPage craft_page;
	InvPage* active_page;

	std::vector<InvPageButton*> page_buttons;
};

class InventoryButton // The bag on the bottom left part of the screen
{
public:
	InventoryButton();
	void Init(Inventory* inventory);

	void Render(sf::RenderTarget& target);
	void HandleEvent(sf::Event const& event);

	bool getOpen() { return open; }

private:
	Inventory* inventory;

	sf::Sprite sprite;
	vec2 pos;
	vec2 size;

	void UpdateOpen();
	bool open = false;
};

class EquippedToolObj // on the bottom right part of the screen
{
public:
	EquippedToolObj();
	void Init(Inventory* inventory, GameState* game_state);

	void Render(sf::RenderTarget& target);
	void HandleEvent(sf::Event const& event);
	void UpdateDurability();

	void setTool(int id);
	int getToolId() { return tool; }

private:
	Inventory* inventory;
	GameState* game_state;

	int tool = -1;
	Item::ItemType tool_type;

	sf::Sprite sprite;
	sf::RectangleShape shape;
	sf::RectangleShape durab_bar;
	vec2 pos;
	vec2 size;
};

class GUIHoverInfo
{
public:
	GUIHoverInfo();
	void Update();
	void Render(sf::RenderTarget& target);
	void setString(const std::string& text);

private:
	std::string text;
	sf::RectangleShape rect;
	sf::Text text_obj;

	vec2 pos;
	vec2 origin;
	vec2 size;

	float margin;

	Tweener rect_alpha_tw;
	Tweener text_alpha_tw;
};
