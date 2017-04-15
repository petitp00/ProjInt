#pragma once

#include "Tweener.h"
#include "Items.h"
#include <SFML/Graphics.hpp>

class GUIObject;
class TextBox;
class TextButton;
class InvItemButton;
class InvActionButton;
class InvPageButton;
class InvToolButton;
class ButtonActionImpl;
struct Controls;

static sf::Color INV_WINDOW_COLOR(115, 106, 91);
static sf::Color INV_ACCENT_COLOR(144, 131, 114);
static sf::Color INV_ACCENT_COLOR2(135, 118, 96);
static sf::Color INV_ACCENT_COLOR3(169, 145, 98);
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

	virtual void Init()=0;
	virtual void Clear();
	virtual void Update()=0;
	virtual void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target)=0;
	virtual bool HandleEvents(sf::Event const& event)=0;

	ButtonActionImpl* button_action_impl = nullptr;
	PosTweener* window_tw = nullptr;
	std::vector<GUIObject*> gui_objects;
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
	void ResetItemDescription(bool item_selected = true);

	int* selected_item;
	std::vector<InvItemButton*> inv_buttons;
	std::vector<InvActionButton*> actions_buttons;
	sf::RectangleShape item_desc_shape;
	TextBox* item_desc_obj;
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
	void ResetItemDescription(bool item_selected = true);
	void UpdateToolsDurability();

	int* selected_tool;
	std::vector<InvToolButton*> inv_buttons;
	std::vector<InvActionButton*> actions_buttons;
	sf::RectangleShape item_desc_shape;
	TextBox* item_desc_obj;
};

struct CraftPage : public InvPage
{
	~CraftPage() override;
	void Init() override;
	void Clear() override;
	void Update() override;
	void Render(sf::RenderTarget& target, sf::RenderTarget& tooltip_render_target) override;
	bool HandleEvents(sf::Event const& event) override;
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
	void ResetItemButtons();
	void ResetItemDescription(bool item_selected = true);
	void UpdateToolsDurability();

	void GoToPage(PageType type);

	// Items
	bool AddItem(int id);
	void AddNewItem(Item::ItemType type);
	void RemoveItem(int id);
	void DeleteItem(int id);
	void EatItem(int id);
	void PutDownItem(int id);

	// Getters
	bool getActive() { return active; }
	bool IsMouseIn(sf::Vector2i mpos);
	std::vector<int>& getItemsId() { return items; }

	// Setters
	void setActive(bool active);

private:
	Controls* controls;
	ButtonActionImpl* button_action_impl;
	bool active = false;
	std::vector<int> items;
	int selected_item;
	int selected_tool;

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
	sf::Vector2f pos;
	sf::Vector2f size;

	void UpdateOpen();
	bool open = false;
};