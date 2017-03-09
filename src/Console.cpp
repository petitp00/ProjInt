#include "Console.h"

#include "ResourceManager.h"

#include <sstream>
#include <iostream>
using namespace std;
using namespace ConsoleNamespace;

bool go_up_then_not_active = false;

float input_height = 30.f;
float margin = 15.f;

Console::Console(Game& game, MenuState& menu_state, GameState& game_state) :
	command_action_impl(*this, game, menu_state, game_state)
{
	Init();
	InitCommands();
}

Console::~Console()
{
	for (int i = 0; i != commands.size(); ++i) {
		delete commands[i];
	}
	commands.clear();
}

void Console::Init()
{
	if (big_mode) CONSOLE_HEIGHT = WINDOW_HEIGHT;
	else CONSOLE_HEIGHT = WINDOW_HEIGHT/3;

	main_shape.setFillColor(BG_COLOR);
	main_shape.setSize({float(WINDOW_WIDTH), float(CONSOLE_HEIGHT)});

	input_shape.setFillColor(BG_COLOR2);
	input_shape.setSize({float(WINDOW_WIDTH), input_height});
	input_shape.setOrigin({0, -float(CONSOLE_HEIGHT - input_height)});

	input_text_obj_greater_then.setFillColor(TEXT_COLOR);
	input_text_obj_greater_then.setCharacterSize(CONSOLE_FONT_SIZE);
	input_text_obj_greater_then.setFont(ResourceManager::getFont(CONSOLE_FONT));
	input_text_obj_greater_then.setOrigin({-margin, -float(CONSOLE_HEIGHT - input_height + CONSOLE_FONT_SIZE/4.f)});
	input_text_obj_greater_then.setString("> ");

	input_text_obj.setFillColor(TEXT_COLOR2);
	input_text_obj.setCharacterSize(CONSOLE_FONT_SIZE);
	input_text_obj.setFont(ResourceManager::getFont(CONSOLE_FONT));
	input_text_obj.setOrigin({-margin - input_text_obj_greater_then.getLocalBounds().width, -float(CONSOLE_HEIGHT - input_height + CONSOLE_FONT_SIZE/4.f)});

	caret_shape.setFillColor(TEXT_COLOR2);
	caret_shape.setSize({2, input_height-8.f});
	caret_shape.setOrigin(0, -float(CONSOLE_HEIGHT - input_height +4.f));

	input_string = " ";
	UpdateInputTextObj();
	UpdateInputCaret(true);
	input_string = "";
	UpdateInputTextObj();

	for (int i = lines.size()-1; i >= 0; --i) {
		lines[i]->setOrigin({-margin, -float(CONSOLE_HEIGHT - input_height - (i+1) * 40.f)});
	}
}

void Console::Update()
{
	ypos = ypos_tw.Tween();

	main_shape.setPosition({0, ypos});
	input_shape.setPosition({0, ypos});
	input_text_obj_greater_then.setPosition({0,ypos});
	input_text_obj.setPosition({0,ypos});
	UpdateInputCaret();

	for (auto & l : lines) {
		l->setPosition({0, ypos});
	}

	if (go_up_then_not_active) {
		if (ypos_tw.getEnded()) {
			active = false;
			go_up_then_not_active = false;
		}
	}
}

void Console::Render(sf::RenderTarget & target)
{
	target.draw(main_shape);
	target.draw(input_shape);
	target.draw(input_text_obj_greater_then);
	target.draw(input_text_obj);
	if (draw_caret) target.draw(caret_shape);
	for (auto & l : lines) {
		target.draw(*l);
	}
}

//
// PARSE AND EXECUTE
//

void Console::ParseAndExecute()
{
	bool found_command = false;

	string str, a;
	vector<string> args;
	stringstream ss(input_string);
	
	ss >> str;
	while (ss >> a) args.push_back(a);

	for (auto c : commands) {
		if (c->name == str) {
			c->action(&command_action_impl, args);
			found_command = true;
		}
	}

	if (!found_command && input_string != "" && input_string != " ") {
		AddLine("\"" + input_string + "\" is not a command.", ERROR);
	}
}

bool Console::HandleEvent(sf::Event const & event)
{
	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::Escape) {
			if (go_up_then_not_active) {
				go_up_then_not_active = false;
				active = false;
			}
			else { setActive(false); }
		}

		if (event.key.code == sf::Keyboard::Return) {
			AddLine("> " + input_string);
			ParseAndExecute();
			input_string = "";
			caret_pos = 0;
		}

		uint input_string_size_before = input_string.size();
		char c = getKeyChar(event.key);
		if (c != 0)
			input_string.insert(caret_pos, {c});
		else if (event.key.code == sf::Keyboard::Space)
			input_string.insert(caret_pos, {" "});
		else if (event.key.code == sf::Keyboard::Dash && event.key.shift)
			input_string.insert(caret_pos, {"_"});
		else if (event.key.code == sf::Keyboard::Dash && !event.key.shift)
			input_string.insert(caret_pos, {"-"});

		else if (event.key.code == sf::Keyboard::BackSpace) {
			if (input_string.size() && caret_pos >0) {
				input_string = input_string.substr(0, caret_pos-1) + input_string.substr(caret_pos);
				--caret_pos;
			}
		}
		else if (event.key.code == sf::Keyboard::Left) {
			if (caret_pos > 0) {
				--caret_pos;
				draw_caret = true;
			}
		}
		else if (event.key.code == sf::Keyboard::Right) {
			if (uint(caret_pos) < input_string.size()) {
				++caret_pos;
				draw_caret = true;
			}
		}


		if (input_string_size_before < input_string.size()) {
			++caret_pos;
		}

		UpdateInputTextObj();
		UpdateInputCaret(true);

		if (event.key.code != sf::Keyboard::F1)
			return true;
	}
	return false;
}

void Console::setActive(bool active)
{
	if (active && !this->active) {
		big_mode = false;
		this->active = true;
		Init();
		ypos_tw.Reset(TweenType::QuartIn, ypos, 0, sf::seconds(0.3f));
	}
	else if (active && this->active) {
		big_mode = !big_mode;
		Init();
		ypos_tw.Reset(TweenType::QuartIn, ypos, 0, sf::seconds(0.3f));
	}
	else {
		big_mode = false;
		go_up_then_not_active = true; // don't set active to false yet because we want to render during the transition
		ypos_tw.Reset(TweenType::QuartIn, ypos, -float(CONSOLE_HEIGHT), sf::seconds(0.3f));
	}
}

void Console::ClearLines()
{
	for (int i = 0; i != lines.size(); ++i) { delete lines[i]; }
	lines.clear();
}

void Console::InitCommands()
{
	#define add_cmd(name, func) commands.push_back(new Command(name, caction_t([](CommandActionImpl* impl, const vector<string>& args) func )));


	add_cmd("test", {
		cout << "it works!" << endl;
	});

	add_cmd("wew", {
		impl->console.AddLine("WEW", RESULT);
	});

	add_cmd("list_args", {
		if (args.size()) {
			impl->console.AddLine("Here are the arguments:", RESULT);
			for (auto & a : args) impl->console.AddLine(a, INFO);
		}
		else {
			impl->console.AddLine("No arguments given", RESULT);
		}
	});

	add_cmd("clear", {
		impl->console.ClearLines();
	});

	add_cmd("help", {
		impl->console.AddLine("This will display available commands", RESULT);
	});

	add_cmd("quit", {
		impl->game.Quit();
	});

	#undef add_cmd
}

void Console::AddLine(std::string text, LineMode mode)
{
	auto l = new sf::Text(text, ResourceManager::getFont(CONSOLE_FONT), CONSOLE_FONT_SIZE);

	if (mode == COMMAND)		l->setFillColor(TEXT_COLOR);
	else if (mode == RESULT)	l->setFillColor(RESULT_COLOR);
	else if (mode == INFO)		l->setFillColor(INFO_COLOR);
	else if (mode == ERROR)		l->setFillColor(ERROR_COLOR);

	lines.push_front(l);

	if (lines.size() > MAX_AMOUNT_OF_LINES) {
		delete lines[lines.size()-1];
		lines.pop_back();
	}

	for (int i = lines.size()-1; i >= 0; --i) {
		lines[i]->setOrigin({-margin, -float(CONSOLE_HEIGHT - input_height - (i+1) * 40.f)});
	}
}

void Console::UpdateInputTextObj()
{
	input_text_obj.setString(input_string);
}

void Console::UpdateInputCaret(bool update_pos)
{
	if (update_pos) {
		float cposx = input_text_obj.findCharacterPos(caret_pos).x;
		caret_shape.setPosition(cposx, ypos);
	}
	else {
		caret_shape.setPosition(caret_shape.getPosition().x, ypos);
	}

	if (caret_clock.getElapsedTime() >= caret_blink_time) {
		caret_clock.restart();
		draw_caret = !draw_caret;
	}
}

