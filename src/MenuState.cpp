#include "MenuState.h"
#include "GameState.h"

#include "ResourceManager.h"

#include <fstream>
#include <iostream>
using namespace std;


static void start_game(ButtonActionImpl* impl) {
	impl->game->ChangeActiveState(State::Game, State::MainMenu);
}

static void quit_game(ButtonActionImpl* impl) {
	impl->game->Quit();
}

// GO TO X MENU //
static void go_to_main_menu(ButtonActionImpl* impl) {
	impl->game->ChangeActiveState(State::MainMenu, State::PauseMenu);
}

static void go_to_new_game_menu(ButtonActionImpl* impl) {
	impl->menu_state->ResetNewGame();
	impl->game->ChangeActiveState(State::NewGameMenu, State::MainMenu);
}

static void go_to_load_game_menu(ButtonActionImpl* impl) {
	impl->menu_state->ResetLoadGame();
	impl->game->ChangeActiveState(State::LoadGameMenu, State::MainMenu);
}

static void go_to_info_menu(ButtonActionImpl* impl) {
	impl->game->ChangeActiveState(State::InfoMenu, State::MainMenu);
}

static void go_to_options_menu(ButtonActionImpl* impl) {
	impl->game->ChangeActiveState(State::OptionsMenu, impl->game->getActiveState());
}

static void go_to_audio_menu(ButtonActionImpl* impl) {
	impl->game->ChangeActiveState(State::AudioOptionsMenu, State::OptionsMenu);
}

static void go_to_controls_menu(ButtonActionImpl* impl) {
	impl->game->ChangeActiveState(State::ControlsOptionsMenu, State::OptionsMenu);
}

static void return_to_last_state(ButtonActionImpl* impl) {
	impl->game->ReturnToLastState();
}

// AUDIO STUFF //
static void toggle_mute_checkbox(ButtonActionImpl* impl) {
	SoundManager::setMute(*impl->mute_active_ref);
}
static void change_volume(ButtonActionImpl* impl) {
	SoundManager::setVolume(*impl->volume_slider_ref);
}

// CONTROLS STUFF //
static void reset_default_controls(ButtonActionImpl* impl) {
	impl->game->getControls().LoadDefault();
	impl->menu_state->ResetControls(impl->game->getControls());
	impl->game->getControls().SaveUserControls();
}

static void set_new_controls(ButtonActionImpl* impl) {
	auto& c = impl->game->getControls();

	for (int i = 0; i != impl->controls_values.size(); ++i) {
		c.keys[i].second = *impl->controls_values[i];
	}

	c.SaveUserControls();
}

// GENERAL STUFF //
static void toggle_fps_checkbox(ButtonActionImpl* impl) {
	impl->game->ToggleFpsCounter();
}

static void create_new_world(ButtonActionImpl* impl) {
	impl->game->ReturnToLastState();
	impl->game_state->StartNewGame(*impl->world_name_ref);
	impl->game->ChangeActiveState(State::Game, State::MainMenu);
}

static void load_world(ButtonActionImpl* impl) {
	impl->game->ReturnToLastState();
	impl->game_state->LoadGame(impl->load_world_name);
	impl->game->ChangeActiveState(State::Game, State::MainMenu);
}

static void delete_world(ButtonActionImpl* impl) {
	vector<string> saves;
	ifstream s("Resources/Data/Saves/all_saves");
	string str;
	char c;
	while (s.get(c)) {
		if (c == '\n') {
			if (str != impl->load_world_name) {
				saves.push_back(str);
			}
			str = "";
		}
		else {
			str += c;
		}
	}
	s.close();


	ofstream s2("Resources/Data/Saves/all_saves");
	for (auto & sv : saves) {
		s2 << sv << endl;
	}
	s2.close();

	std::string sss = "del \"Resources\\Data\\Saves\\" + impl->load_world_name + "\"";
	system(sss.c_str());

	ifstream back("Resources/Data/saves/" + impl->load_world_name + ".backup");
	if (back) {
		std::string ssss = "del \"Resources\\Data\\Saves\\" + impl->load_world_name + ".backup\"";
		back.close();
		system(ssss.c_str());
	}

	impl->menu_state->ResetLoadGame();
}

// MENU PAGE
MenuPage::MenuPage()
{
	tooltip_render_target.create(WINDOW_WIDTH, WINDOW_HEIGHT);
	tooltip_render_target.setSmooth(true);
	tooltip_render_target_sprite.setTexture(tooltip_render_target.getTexture());
}

MenuPage::~MenuPage() { for (auto o : gui_objects) { delete o; } }

void MenuPage::Clear()
{
	for (int i = 0; i != gui_objects.size(); ++i) {
		delete gui_objects[i];
	}
	gui_objects.clear();
}

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

bool MenuPage::MousePressedEvent(int mouse_x, int mouse_y)
{
	bool ret = false;
	vec2i mouse(mouse_x, mouse_y);
	for (auto o : gui_objects) {
		if (o->getHovered()) {
			if (o->onClick(mouse)) { ret = true; return true; }
		}
	}
	return ret;
}

bool MenuPage::MouseReleasedEvent(int mouse_x, int mouse_y)
{
	bool ret = false;
	for (auto o : gui_objects) {
		if (o->isClicked()) {
			if (o->onClickRelease()) ret = true;
		}
	}
	return ret;
}

bool MenuPage::MouseMovedEvent(int mouse_x, int mouse_y)
{
	bool ret = false;
	vec2i mouse(mouse_x, mouse_y);
	for (auto o : gui_objects) {
		if (o->isClicked()) {
			o->UpdateClickDrag(mouse);
		}
		if (o->isMouseIn(mouse)) {
			if (!o->getHovered()) {
				if (o->onHoverIn(mouse)) ret = true;
			}
			else {
				o->UpdateHoveredMousePos(mouse);
			}
		}
		else if (o->getHovered()) {
			if (o->onHoverOut()) ret = true;
		}
	}
	return ret;
}

bool MenuPage::MouseWheelScrolledEvent(float delta)
{
	bool ret = false;
	for (auto o : gui_objects) {
		if (o->getHovered()) {
			if (o->onMouseWheel(delta)) ret = true;
		}
	}
	return ret;
}

bool MenuPage::KeyPressedEvent(sf::Event::KeyEvent e)
{
	bool ret = false;
	for (auto o : gui_objects) {
		if (o->onKeyType(e)) ret = true;
	}
	return ret;
}

// MENU STATE
MenuState::MenuState(Game& game) :
	//button_action_impl(game, *this, game.getGameState()),
	active_page(&main_menu)
{
}

MenuState::~MenuState()
{
}

void MenuState::Init(ButtonActionImpl * button_action_impl)
{
	this->button_action_impl = button_action_impl;
	InitMainMenu();
	InitNewGameMenu();
	InitLoadGameMenu();
	InitInfoMenu();
	InitOptionsMenu();
	InitAudioMenu();
	InitControlsMenu(button_action_impl->game->getControls());
	InitPauseMenu();
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

bool MenuState::HandleEvents(sf::Event const & event)
{
	vec2i mouse_pos = vec2i(button_action_impl->game->getMousePos());
	if (event.type == sf::Event::KeyPressed) {
		return active_page->KeyPressedEvent(event.key);
	}
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Button::Left)
			return active_page->MousePressedEvent(mouse_pos.x, mouse_pos.y);
	}
	if (event.type == sf::Event::MouseButtonReleased) {
		if (event.mouseButton.button == sf::Mouse::Button::Left)
			return active_page->MouseReleasedEvent(mouse_pos.x, mouse_pos.y);
	}
	if (event.type == sf::Event::MouseMoved) {
		return active_page->MouseMovedEvent(mouse_pos.x, mouse_pos.y);
	}
	if (event.type == sf::Event::MouseWheelScrolled) {
		return active_page->MouseWheelScrolledEvent(event.mouseWheelScroll.delta);
	}
	return false;
}

void MenuState::InitMainMenu()
{
	auto title = new TextBox("Projet Intégrateur", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	main_menu.AddGUIObject(title);

	float button_width = title->getSize().x;

	auto play_button = new TextButton("Nouvelle partie", {100, 250}, button_width);
	play_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_new_game_menu), button_action_impl);
	play_button->setTooltip(new Tooltip("Commencer une partie", sf::seconds(0.55f)));
	main_menu.AddGUIObject(play_button);

	auto load_button = new TextButton("Charger une partie", {100, 350}, button_width);
	load_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_load_game_menu), button_action_impl);
	load_button->setTooltip(new Tooltip("Charger une partie sauvegardée", sf::seconds(0.55f)));
	main_menu.AddGUIObject(load_button);

	auto options_button = new TextButton("Options", {100, 450}, button_width / 2.f - 20.f);
	options_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_options_menu), button_action_impl);
	options_button->setTooltip(new Tooltip("Accéder aux options", sf::seconds(0.55f)));
	main_menu.AddGUIObject(options_button);

	auto info_button = new TextButton("Informations", {100 + button_width / 2.f + 20.f, 450}, button_width / 2.f - 20.f);
	info_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_info_menu), button_action_impl);
	info_button->setTooltip(new Tooltip("Informations sur le projet et ses créateurs", sf::seconds(0.55f)));
	main_menu.AddGUIObject(info_button);

	auto quit_button = new TextButton("Quitter", {100, 550}, button_width);
	quit_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(quit_game), button_action_impl);
	quit_button->setTooltip(new Tooltip("Quitter le jeu", sf::seconds(0.55f)));
	main_menu.AddGUIObject(quit_button);
}

void MenuState::InitNewGameMenu()
{
	auto title = new TextBox("Nouvelle Partie", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	new_game_menu.AddGUIObject(title);

	auto name_label = new TextBox("Nommer la partie:", {100, 350}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::NORMAL);
	new_game_menu.AddGUIObject(name_label);

	float px = name_label->getPos().x + name_label->getSize().x + 40.f;
	auto name_input = new TextInputBox({px, 355.f}, float(WINDOW_WIDTH)- px - 80.f - 65.f);
	name_input->setOnClickAction(new std::function<void(ButtonActionImpl*)>(create_new_world), button_action_impl);
	name_input->setActive(true);
	button_action_impl->world_name_ref = name_input->getStringRef();
	new_game_menu.AddGUIObject(name_input);

	auto ok_button = new TextButton("Ok", {float(WINDOW_WIDTH) - 65.f -60.f, 347.f}, 65.f, FontSize::SMALL);
	ok_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(create_new_world), button_action_impl);
	new_game_menu.AddGUIObject(ok_button);

	auto return_button = new TextButton("Annuler", {float(WINDOW_WIDTH - 220), float(WINDOW_HEIGHT - 100)}, 0);
	return_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), button_action_impl);
	new_game_menu.AddGUIObject(return_button);
}

void MenuState::InitLoadGameMenu()
{
	auto title = new TextBox("Charger une partie", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	load_game_menu.AddGUIObject(title);

	vector<string> saves;
	ifstream s("Resources/Data/Saves/all_saves");
	string str;
	char c;
	while (s.get(c)) {
		if (c == '\n') {
			saves.push_back(str);
			str = "";
		}
		else {
			str += c;
		}
	}
	float w = float(WINDOW_WIDTH - 200);

	auto container = new ObjContainer({100, 180}, {w, 420.f});
	int i = 0;
	for (auto & sv : saves) {
		string sss = sv;
		if (sv.size() >= 16) {
			sss = sv.substr(0, 16) + "...";
		}
		auto label = new TextBox(sss, {20, float(20 + 100*i)}, w, BASE_FONT_NAME, sf::Color::Black, FontSize::LARGE);
		container->AddObject(label);

		auto butt = new WorldSelectButton("Jouer", {650.f, float(26 + 100*i)}, 130.f, FontSize::SMALL, sf::Color::Black);
		butt->setWorldName(sv);
		butt->setOnClickAction(new std::function<void(ButtonActionImpl*)>(load_world), button_action_impl);
		container->AddObject(butt);

		auto butt2 = new WorldSelectButton("Supprimer", {130.f + 20.f + 650.f, float(26 + 100*i)}, 200.f, FontSize::SMALL, sf::Color::Black);
		butt2->setWorldName(sv);
		butt2->setOnClickAction(new std::function<void(ButtonActionImpl*)>(delete_world), button_action_impl);
		container->AddObject(butt2);

		++i;
	}

	load_game_menu.AddGUIObject(container);

	auto return_button = new TextButton("Retour", {float(WINDOW_WIDTH - 220), float(WINDOW_HEIGHT - 100)}, 0);
	return_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), button_action_impl);
	load_game_menu.AddGUIObject(return_button);
}

void MenuState::InitInfoMenu()
{
	auto title = new TextBox("Informations", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	info_menu.AddGUIObject(title);

	auto return_button = new TextButton("Retour", {float(WINDOW_WIDTH - 220), float(WINDOW_HEIGHT - 100)}, 0);
	return_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), button_action_impl);
	info_menu.AddGUIObject(return_button);
}

void MenuState::InitOptionsMenu()
{
	auto title = new TextBox("Options", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	options_menu.AddGUIObject(title);

	auto audio_button = new TextButton("Audio", {100, 250}, 744);
	audio_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_audio_menu), button_action_impl);
	options_menu.AddGUIObject(audio_button);

	auto controls_button = new TextButton("Contrôles", {100, 350}, 744);
	controls_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_controls_menu), button_action_impl);
	options_menu.AddGUIObject(controls_button);

	auto fps_label = new TextBox("Compteur de FPS:", {100, 450}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::NORMAL);
	options_menu.AddGUIObject(fps_label);

	auto fps_checkbox = new Checkbox(true, {804, 455});
	fps_checkbox->setOnClickAction(new std::function<void(ButtonActionImpl*)>(toggle_fps_checkbox), button_action_impl);
	fps_checkbox->setTooltip(new Tooltip("Affiche le nombre d'images par secondes. (Coin supérieur gauche)", sf::seconds(0.55f)));
	options_menu.AddGUIObject(fps_checkbox);

	auto return_button = new TextButton("Retour", {float(WINDOW_WIDTH - 220), float(WINDOW_HEIGHT - 100)}, 0);
	return_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), button_action_impl);
	options_menu.AddGUIObject(return_button);
}

void MenuState::InitAudioMenu()
{
	auto title = new TextBox("Options > Audio", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	audio_menu.AddGUIObject(title);

	auto mute_label = new TextBox("Sourdine:", {100, 250}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::NORMAL);
	audio_menu.AddGUIObject(mute_label);

	auto mute_checkbox = new Checkbox(false, {375, 255});
	mute_checkbox->setOnClickAction(new std::function<void(ButtonActionImpl*)>(toggle_mute_checkbox), button_action_impl);
	button_action_impl->mute_active_ref = mute_checkbox->getActiveRef();
	audio_menu.AddGUIObject(mute_checkbox);

	auto volume_label = new TextBox("Volume:", {100, 350}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::NORMAL);
	audio_menu.AddGUIObject(volume_label);

	auto volume_slider = new Slider({300, 355}, 500, 75, 0, 100);
	button_action_impl->volume_slider_ref = volume_slider->getValueRef();
	volume_slider->setOnClickAction(new std::function<void(ButtonActionImpl*)>(change_volume), button_action_impl);
	audio_menu.AddGUIObject(volume_slider);

	SoundManager::setVolume(*button_action_impl->volume_slider_ref);

	auto return_button = new TextButton("Retour", {float(WINDOW_WIDTH - 220), float(WINDOW_HEIGHT - 100)}, 0);
	return_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), button_action_impl);
	audio_menu.AddGUIObject(return_button);
}

void MenuState::InitControlsMenu(Controls& controls)
{
	button_action_impl->controls_values.clear();

	auto title = new TextBox("Options > Contrôles", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	controls_menu.AddGUIObject(title);

	auto reset_button = new TextButton("Réinitialiser", {700, float(WINDOW_HEIGHT - 100)}, 0);
	reset_button->setTooltip(new Tooltip("Réinitialiser aux valeur par défaut", sf::seconds(0.55f)));
	reset_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(reset_default_controls), button_action_impl);
	controls_menu.AddGUIObject(reset_button);

	float w = float(WINDOW_WIDTH - 200);
	auto container = new ObjContainer({100, 180}, {w, 420.f});

	int i = 0;
	for (auto k : controls.keys) {
		auto label = new TextBox(k.first + ":", {20, float(20 + 100*i)}, w, BASE_FONT_NAME, sf::Color::Black, FontSize::LARGE);
		container->AddObject(label);

		auto butt = new ControlsTextButton(getKeyString(k.second), {40 + w/2.f, float(20+100*i)}, w/2.f - 160.f, 40);
		butt->setOnClickAction(new std::function<void(ButtonActionImpl*)>(set_new_controls), button_action_impl);
		butt->setKey(k.second);
		button_action_impl->controls_values.push_back(butt->getKeyRef());

		container->AddObject(butt);

		++i;
	}

	controls_menu.AddGUIObject(container);

	auto return_button = new TextButton("Retour", {float(WINDOW_WIDTH - 220), float(WINDOW_HEIGHT - 100)}, 0);
	return_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), button_action_impl);
	controls_menu.AddGUIObject(return_button);
}

void MenuState::InitPauseMenu()
{
	auto title = new TextBox("Pause", {100, 80}, float(WINDOW_WIDTH), BASE_FONT_NAME, sf::Color::Black, FontSize::BIG);
	pause_menu.AddGUIObject(title);

	auto resume_button = new TextButton("Retour au jeu", {100, 250}, 744);
	resume_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(return_to_last_state), button_action_impl);
	pause_menu.AddGUIObject(resume_button);

	auto options_button = new TextButton("Options", {100, 350}, 744);
	options_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_options_menu), button_action_impl);
	pause_menu.AddGUIObject(options_button);

	auto menu_button = new TextButton("Retour au menu principal", {100, 450}, 744);
	menu_button->setOnClickAction(new std::function<void(ButtonActionImpl*)>(go_to_main_menu), button_action_impl);
	pause_menu.AddGUIObject(menu_button);
}
