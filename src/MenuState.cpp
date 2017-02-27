#include "MenuState.h"

#include "ResourceManager.h"

#include <iostream>
using namespace std;


static void start_game(ButtonActionImpl* impl) {
	impl->game.ChangeActiveState(State::Game, State::MainMenu);
}

static void quit_game(ButtonActionImpl* impl) {
	impl->game.Quit();
}

static void go_to_info_menu(ButtonActionImpl* impl) {
	impl->game.ChangeActiveState(State::InfoMenu, State::MainMenu);
}

static void go_to_options_menu(ButtonActionImpl* impl) {
	impl->game.ChangeActiveState(State::OptionsMenu, State::MainMenu);
}

static void go_to_audio_menu(ButtonActionImpl* impl) {
	impl->game.ChangeActiveState(State::AudioOptionsMenu, State::OptionsMenu);
}

static void go_to_controls_menu(ButtonActionImpl* impl) {
	impl->game.ChangeActiveState(State::ControlsOptionsMenu, State::OptionsMenu);
}

static void return_to_last_state(ButtonActionImpl* impl) {
	impl->game.ReturnToLastState();
}

static void toggle_mute_checkbox(ButtonActionImpl* impl) { }
static void change_volume(ButtonActionImpl* impl) { }

static void toggle_fps_checkbox(ButtonActionImpl* impl) {
	impl->game.ToggleFpsCounter();
}

// MENU PAGE
MenuPage::MenuPage()
{
	tooltip_render_target.create(WINDOW_WIDTH, WINDOW_HEIGHT);
	tooltip_render_target.setSmooth(true);
	tooltip_render_target_sprite.setTexture(tooltip_render_target.getTexture());
}

MenuPage::~MenuPage() { for (auto o : gui_objects) { delete o; } }

void MenuPage::Update() { for (auto o : gui_objects) { o->Update(); } }

void MenuPage::Render(sf::RenderTarget& target)
{
	tooltip_render_target.clear(sf::Color::Transparent);

	for (auto o : gui_objects) {
		o->Render(target, tooltip_render_target);
	}

	tooltip_render_target.display();
	target.draw(tooltip_render_target_sprite);
}

void MenuPage::MousePressedEvent(int mouse_x, int mouse_y)
{
	for (auto o : gui_objects) {
		if (o->getHovered()) {
			o->onClick();
		}
	}
}

void MenuPage::MouseReleasedEvent(int mouse_x, int mouse_y)
{
	for (auto o : gui_objects) {
		if (o->isClicked()) {
			o->onClickRelease();
		}
	}
}

void MenuPage::MouseMovedEvent(int mouse_x, int mouse_y)
{
	sf::Vector2i mouse(mouse_x, mouse_y);
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
}

void MenuPage::MouseWheelScrolledEvent(float delta)
{
	for (auto o : gui_objects) {
		if (o->getHovered()) {
			o->onMouseWheel(delta);
		}
	}
}

// MENU STATE
MenuState::MenuState(Game& game) :
	button_action_impl(game),
	active_page(&main_menu)
{
	InitMainMenu();
	InitInfoMenu();
	InitOptionsMenu();
	InitAudioMenu();
	InitControlsMenu(game.getControls());
}

MenuState::~MenuState()
{
}

void MenuState::Update()
{
	if (active) {
		active_page->Update();
	}
}

void MenuState::Render(sf::RenderTarget & target)
{
	target.clear(sf::Color(183, 241, 244));

	active_page->Render(target);
}

void MenuState::HandleEvents(sf::Event const & event)
{
	if (event.type == sf::Event::KeyPressed) {
		//active_page->KeyPressedEvent(event.key.code);
	}
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Button::Left)
			active_page->MousePressedEvent(event.mouseButton.x, event.mouseButton.y);
	}
	if (event.type == sf::Event::MouseButtonReleased) {
		if (event.mouseButton.button == sf::Mouse::Button::Left)
			active_page->MouseReleasedEvent(event.mouseButton.x, event.mouseButton.y);
	}
	if (event.type == sf::Event::MouseMoved) {
		active_page->MouseMovedEvent(event.mouseMove.x, event.mouseMove.y);
	}
	if (event.type == sf::Event::MouseWheelScrolled) {
		active_page->MouseWheelScrolledEvent(event.mouseWheelScroll.delta);
	}
}

void MenuState::KeyPressedEvent(sf::Keyboard::Key key)
{
}

void MenuState::MousePressedEvent(sf::Mouse::Button button, int mouse_x, int mouse_y)
{
	active_page->MousePressedEvent(mouse_x, mouse_y);
}

void MenuState::MouseReleasedEvent(sf::Mouse::Button button, int mouse_x, int mouse_y)
{
	active_page->MouseReleasedEvent(mouse_x, mouse_y);
}

void MenuState::MouseMovedEvent(int mouse_x, int mouse_y)
{
	active_page->MouseMovedEvent(mouse_x, mouse_y);
}

void MenuState::InitMainMenu()
{
	auto title = new TextBox("Projet Intégrateur", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	main_menu.AddGUIObject(title);

	float button_width = title->getSize().x;

	auto play_button = new TextButton("Jouer", {100, 250}, button_width);
	play_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(start_game), &button_action_impl);
	play_button->setTooltip(new Tooltip("Commencer une partie", sf::seconds(0.55f)));
	main_menu.AddGUIObject(play_button);

	auto load_button = new TextButton("Charger une partie", {100, 350}, button_width);
	load_button->setTooltip(new Tooltip("Charger une partie sauvegardée", sf::seconds(0.55f)));
	main_menu.AddGUIObject(load_button);

	auto options_button = new TextButton("Options", {100, 450}, button_width / 2.f - 20.f);
	options_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_options_menu), &button_action_impl);
	options_button->setTooltip(new Tooltip("Accéder aux options", sf::seconds(0.55f)));
	main_menu.AddGUIObject(options_button);

	auto info_button = new TextButton("Informations", {100 + button_width / 2.f + 20.f, 450}, button_width / 2.f - 20.f);
	info_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_info_menu), &button_action_impl);
	info_button->setTooltip(new Tooltip("Informations sur le projet et ses créateurs", sf::seconds(0.55f)));
	main_menu.AddGUIObject(info_button);

	auto quit_button = new TextButton("Quitter", {100, 550}, button_width);
	quit_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(quit_game), &button_action_impl);
	quit_button->setTooltip(new Tooltip("Quitter le jeu", sf::seconds(0.55f)));
	main_menu.AddGUIObject(quit_button);
}

void MenuState::InitInfoMenu()
{
	auto title = new TextBox("Informations", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	info_menu.AddGUIObject(title);

	auto return_button = new TextButton("Retour", {float(WINDOW_WIDTH - 220), float(WINDOW_HEIGHT - 100)}, 0);
	return_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), &button_action_impl);
	info_menu.AddGUIObject(return_button);
}

void MenuState::InitOptionsMenu()
{
	auto title = new TextBox("Options", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	options_menu.AddGUIObject(title);

	auto audio_button = new TextButton("Audio", {100, 250}, 744);
	audio_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_audio_menu), &button_action_impl);
	options_menu.AddGUIObject(audio_button);

	auto controls_button = new TextButton("Contrôles", {100, 350}, 744);
	controls_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_controls_menu), &button_action_impl);
	options_menu.AddGUIObject(controls_button);

	auto fps_label = new TextBox("Compteur de FPS:", {100, 450}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::NORMAL);
	options_menu.AddGUIObject(fps_label);

	auto fps_checkbox = new Checkbox(true, {804, 455});
	fps_checkbox->setOnClickAction(new std::function<void(ButtonActionImpl*)>(toggle_fps_checkbox), &button_action_impl);
	fps_checkbox->setTooltip(new Tooltip("Affiche le nombre d'images par secondes. (Coin supérieur gauche)", sf::seconds(0.55f)));
	options_menu.AddGUIObject(fps_checkbox);

	auto return_button = new TextButton("Retour", {float(WINDOW_WIDTH - 220), float(WINDOW_HEIGHT - 100)}, 0);
	return_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), &button_action_impl);
	options_menu.AddGUIObject(return_button);
}

void MenuState::InitAudioMenu()
{
	auto title = new TextBox("Options > Audio", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	audio_menu.AddGUIObject(title);

	auto mute_label = new TextBox("Sourdine:", {100, 250}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::NORMAL);
	audio_menu.AddGUIObject(mute_label);

	auto mute_checkbox = new Checkbox(false, {375, 255});
	mute_checkbox->setOnClickAction(new std::function<void(ButtonActionImpl*)>(toggle_mute_checkbox), &button_action_impl);
	button_action_impl.mute_active_ref = mute_checkbox->getActiveRef();
	audio_menu.AddGUIObject(mute_checkbox);

	auto volume_label = new TextBox("Volume:", {100, 350}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::NORMAL);
	audio_menu.AddGUIObject(volume_label);

	auto volume_slider = new Slider({300, 355}, 500, 75, 0, 100);
	button_action_impl.volume_slider_ref = volume_slider->getValueRef();
	volume_slider->setOnClickAction(new std::function<void(ButtonActionImpl*)>(change_volume), &button_action_impl);
	audio_menu.AddGUIObject(volume_slider);

	auto return_button = new TextButton("Retour", {float(WINDOW_WIDTH - 220), float(WINDOW_HEIGHT - 100)}, 0);
	return_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), &button_action_impl);
	audio_menu.AddGUIObject(return_button);
}

void MenuState::InitControlsMenu(Controls& controls)
{
	auto title = new TextBox("Options > Contrôles", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	controls_menu.AddGUIObject(title);

	auto reset_button = new TextButton("Réinitialiser", {700, float(WINDOW_HEIGHT - 100)}, 0);
	reset_button->setTooltip(new Tooltip("Réinitialiser aux valeur par défaut", sf::seconds(0.55f)));
	controls_menu.AddGUIObject(reset_button);

	float w = float(WINDOW_WIDTH - 200);
	auto container = new ObjContainer({100, 180}, {w, 420.f});

	int i = 0;
	for (auto k : controls.keys) {
		auto label = new TextBox(k.first + ":", {20, float(20 + 100*i)}, w, BASE_FONT_NAME, sf::Color::Black, FontSize::LARGE);
		container->AddObject(label);

		auto butt = new TextButton(getKeyString(k.second), {40 + w/2.f, float(20+100*i)}, w/2.f - 160.f);
		container->AddObject(butt);

		++i;
	}

	controls_menu.AddGUIObject(container);

	auto return_button = new TextButton("Retour", {float(WINDOW_WIDTH - 220), float(WINDOW_HEIGHT - 100)}, 0);
	return_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), &button_action_impl);
	controls_menu.AddGUIObject(return_button);
}
