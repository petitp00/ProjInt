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

	caret_pos = 0;
	input_string = " ";
	UpdateInputTextObj();
	UpdateInputCaret(true);
	input_string = "";
	UpdateInputTextObj();

	UpdateLinesOrigin();
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

void Console::PrintInfo(const std::string & str)
{
	AddLine(str, INFO);
}

Command* Console::getCommand(const std::string & name)
{
	for (auto c : commands) {
		if (c->name == name) return c;
	}
	return nullptr;
}

void Console::setActive(bool active)
{
	if (active && !this->active) {
		big_mode = false;
		this->active = true;
		Init();
		ypos_tw.Reset(TweenType::QuartInOut, ypos, 0, sf::seconds(0.3f));
	}
	else if (active && this->active) {
		big_mode = !big_mode;
		Init();
		ypos_tw.Reset(TweenType::QuartInOut, ypos, 0, sf::seconds(0.3f));
	}
	else {
		big_mode = false;
		go_up_then_not_active = true; // don't set active to false yet because we want to render during the transition
		ypos_tw.Reset(TweenType::QuartInOut, ypos, -float(CONSOLE_HEIGHT), sf::seconds(0.3f));
	}
}

void Console::ClearLines()
{
	for (int i = 0; i != lines.size(); ++i) { delete lines[i]; }
	lines.clear();
}

void Console::AddInfo(const std::string & name, const std::string & desc, const std::string & help)
{
	auto c = getCommand(name);
	if (!c) {
		cerr << "Command \"" << name << "\" does not exist. AddInfo() failed" << endl;
		return;
	}
	c->desc = desc;
	c->help_string = help;
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
	AddInfo("clear", "Clear the console", "Just write clear. No arguments needed.");

	add_cmd("help", {
		if (!args.size()) {
			for (auto c : impl->console.getCommands()) {
				int nb_of_spaces = 30 - c->name.size();
				string str = c->name;
				while (nb_of_spaces--) str+=' ';
				str += c->desc;
				impl->console.AddLine(str, RESULT);
			}
		}
		else if (args.size() == 1) {
			std::string name = args[0];
			auto c = impl->console.getCommand(name);
			if (c)	impl->console.AddLine(c->help_string					 , RESULT);
			else	impl->console.AddLine("\"" + name + "\" is not a command", ERROR);
		}
		else {
			impl->console.AddLine("Too many arguments given. Expected 0 or 1", ERROR);
		}
	});
	AddInfo("help", "Display available commands or help for specific command",
			"Usage:\nhelp            List commands\nhelp command    Display help for command"
	);

	add_cmd("quit", {
		impl->game.Quit();
	});
	AddInfo("quit", "Quit game", "Just write quit. No arguments needed.");

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

	UpdateLinesOrigin();
}

void Console::UpdateLinesOrigin()
{
	if (lines.size()) {
		lines[0]->setOrigin(-margin, -float(CONSOLE_HEIGHT - input_height - 20.f - lines[0]->getLocalBounds().height));
		for (int i = 1; i < lines.size(); ++i) {
			lines[i]->setOrigin(-margin, -float(-lines[i-1]->getOrigin().y - 18.f - lines[i]->getLocalBounds().height));
		}
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

