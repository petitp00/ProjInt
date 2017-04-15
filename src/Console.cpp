#include "Console.h"

#include "ResourceManager.h"
#include "GameGUI.h"
#include "GameState.h"

#include <fstream>
#include <sstream>
#include <iostream>


using namespace std;
using namespace ConsoleNamespace;

bool go_up_then_not_active = false;

float input_height = 30.f;
float margin = 15.f;

Console::Console(Game& game, MenuState& menu_state, GameState& game_state) :
	command_action_impl(this, &game, &menu_state, &game_state)
{
	Init();
	InitCommands();
}

Console::Console(EditorMode::Editor & editor) :
	command_action_impl(this, &editor)
{
	Init();
	InitEditorCommands();
}

Console::~Console()
{
	for (int i = 0; i != commands.size(); ++i) {
		delete commands[i];
	}
	commands.clear();
}

void Console::Init(bool reset_input_string)
{
	if (big_mode) CONSOLE_HEIGHT = WINDOW_HEIGHT;
	else CONSOLE_HEIGHT = WINDOW_HEIGHT/2;

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

	if (reset_input_string) {
		caret_pos = 0;
		input_string = " ";
		UpdateInputTextObj();
		UpdateInputCaret(true);
		input_string = "";
	}
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
	if (draw_caret && typing_active) target.draw(caret_shape);
	for (auto & l : lines) {
		target.draw(*l);
	}
}

//
// PARSE AND EXECUTE
//

string getLower(const string& str) {
	string s;
	for (auto c : str) {
		s+=tolower(c);
	}
	return s;
}

void Console::ParseAndExecute()
{
	bool found_command = false;

	string str, a;
	vector<string> args;
	stringstream ss(input_string);
	
	ss >> str;
	while (ss >> a) args.push_back(a);

	for (auto c : commands) {
		if (getLower(c->name) == getLower(str)) {
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
	if (event.type == sf::Event::MouseButtonPressed) {
		if (true || event.mouseButton.button == sf::Mouse::Left) {
			auto mx = event.mouseButton.x;
			auto my = event.mouseButton.y;
			if (!big_mode) {
				if (my > CONSOLE_HEIGHT) {
					typing_active = false;
				}
				else {
					typing_active = true;
				}
			}
		}
		return true;
	}

	if (event.type == sf::Event::KeyPressed) {
		if (event.key.code == sf::Keyboard::Escape) {
			if (go_up_then_not_active) {
				go_up_then_not_active = false;
				active = false;
			}
			else { setActive(false); }
			return true;
		}

		if (typing_active) {
			bool up_or_down = false;
			if (event.key.code == sf::Keyboard::Return) {
				AddLine("> " + input_string);
				command_history.push_front(input_string);
				c_hist_index = -1;
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
			else if (event.key.code == sf::Keyboard::Up) {
				if (command_history.size()) {
					if (c_hist_index < (int)command_history.size()-1) { ++c_hist_index;}
					if (c_hist_index >= 0) input_string = command_history[c_hist_index];
				}
				up_or_down = true;
			}
			else if (event.key.code == sf::Keyboard::Down) {
				if (command_history.size()) {
					if (c_hist_index > 0) { --c_hist_index; }
					if (c_hist_index >= 0) input_string = command_history[c_hist_index];
				}
				up_or_down = true;
			}


			if (input_string_size_before < input_string.size() && !up_or_down) {
				++caret_pos;
			}
			if (up_or_down) {
				caret_pos = input_string.size();
			}

			UpdateInputTextObj();
			UpdateInputCaret(true);

		}
		if (event.key.code != sf::Keyboard::F1 && typing_active)
			return true;

	}
	return false;
}

void Console::PrintInfo(const std::string & str)
{
	AddLine(str, INFO);
}

void Console::UpdateResize()
{
	CONSOLE_HEIGHT = WINDOW_HEIGHT/3;
	Init(false);
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
		this->typing_active = true;
		Init();
		ypos_tw.Reset(TweenType::QuartInOut, ypos, 0, sf::seconds(0.3f));
	}
	else if (active && this->active) {
		big_mode = !big_mode;
		this->typing_active = true;
		Init(false);
		ypos_tw.Reset(TweenType::QuartInOut, ypos, 0, sf::seconds(0.3f));
	}
	else {
		big_mode = false;
		this->typing_active = false;
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
		for (uint i = 1; i < lines.size(); ++i) {
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

// INIT COMMANDS // INIT COMMANDS // INIT COMMANDS // INIT COMMANDS // INIT COMMANDS // INIT COMMANDS // INIT COMMANDS // INIT COMMANDS //

void Console::InitCommands()
{
	#define add_cmd(name, func) commands.push_back(new Command(name, caction_t([](CommandActionImpl* impl, const vector<string>& args) func )));

	add_cmd("give", {
		if (args.size() >= 1) {
			auto name = args[0];
			if (name[0] == '"') {
				name = name.substr(1, name.size());
				if (name[name.size()-1] == '"') {
					name = name.substr(0, name.size()-1);
				}
				else {
					for (int i = 1; i != args.size(); ++i) {
						name += ' ';
						auto n = args[i];
						if (n[n.size()-1] == '"') {
							name += n.substr(0, n.size()-1);
							break;
						}
						else {
							name += args[i];
						}
					}
				}

			}
			auto it = Item::getItemTypeByName(name);
			if (it != -1) {
				auto i = Item::Manager::CreateItem(it);
				impl->game_state->getInventory()->AddItem(i);
			}
			else {
				impl->console->AddLine("\"" + name + "\" is not an item", ERROR);
			}
		}
		else {
			impl->console->AddLine("Usage: give \"item_name\"", ERROR);
		}
	});

	add_cmd("clear", {
		impl->console->ClearLines();
	});
	AddInfo("clear", "Clear the console", "Just write clear. No arguments needed.");

	add_cmd("help", {
		if (!args.size()) {
			for (auto c : impl->console->getCommands()) {
				int nb_of_spaces = 30 - c->name.size();
				string str = c->name;
				while (nb_of_spaces--) str+=' ';
				str += c->desc;
				impl->console->AddLine(str, RESULT);
			}
		}
		else if (args.size() == 1) {
			std::string name = args[0];
			auto c = impl->console->getCommand(name);
			if (c)	impl->console->AddLine(c->help_string					 , RESULT);
			else	impl->console->AddLine("\"" + name + "\" is not a command", ERROR);
		}
		else {
			impl->console->AddLine("Too many arguments given. Expected 0 or 1", ERROR);
		}
	});
	AddInfo("help", "Display available commands or help for specific command",
			"Usage:\nhelp            List commands\nhelp command    Display help for command"
	);

	add_cmd("quit", {
		impl->game->Quit();
	});
	AddInfo("quit", "Quit game", "Just write quit. No arguments needed.");

	#undef add_cmd
}

void Console::InitEditorCommands()
{
	#define add_cmd(name, func) commands.push_back(new Command(name, caction_t([&](CommandActionImpl* impl, const vector<string>& args) func )));

	add_cmd("load", {
		if (args.size() == 1) {
			impl->editor->world.LoadWorld(args[0]);
		}
		else {
			string s_str = "Available saves:";
			string s;
			ifstream ss("Resources/Data/Saves/all_saves");
			while (ss >> s) s_str += "\n" + s;
			impl->editor->console->AddLine(s_str, RESULT);
		}
	});
	AddInfo("load", "Load an existing save", "Usage:\n\
load                 Display list of available saves\n\
load world_name      Load world_name");

	add_cmd("dev", {
		impl->editor->world.CreateNewBlank("dev");
		impl->editor->world_init = true;
	});
	AddInfo("dev", "Quick create world \"dev\"", "Just write dev. Overwrites the previous world.");

	add_cmd("save", {
		if (args.size() == 0) {
			impl->editor->world.Save();
		}
		else {
			impl->editor->world.Save(args[0]);
		}
	});
	AddInfo("save", "Save world", "Usage:\nsave                     save the world\nsave world_name          save the world to world_name");

	add_cmd("CreateWorld", {
		if (args.size() == 0)
			impl->console->AddLine("Usage:\nCreateWorld world_name", ERROR);
		else if (args.size() == 1) {
			impl->editor->world.CreateNewBlank(args[0]);
			impl->editor->world_init = true;
		}
		else impl->console->AddLine("Too many arguments given.", ERROR);
	});
	AddInfo("CreateWorld", "Creates new world.", "Usage:\nCreateWorld world_name");

	add_cmd("ground", {
		impl->editor->ToggleGroundEditMode();
	});
	AddInfo("ground", "Toggles editor's ground edit mode", "Just type ground");

	#define new_ent_usage "Usage:\n\
NewEnt Type              Creates an entity of type [Type] at (0,0) with no flags (or with default flags)\n\
NewEnt Type flags        Creates an entity of type [Type] at (0,0) with flags (added to default flags if they exist)\n\
NewEnt Type x y          Creates an entity of type [Type] at (x,y) with no flags (or with default flags)\n\
NewEnt Type x y flags    Creates an entity of type [Type] at (x,y) with flags (added to default flags if they exist)\n\
"
	
	add_cmd("NewEnt", {
		if (args.size() == 0) {
			impl->console->AddLine(new_ent_usage, ERROR);
			return;
		}
	
		Type type = ENTITY;
		vec2 pos(0, 0);
		unsigned long flags = NO_FLAG;


		if (args.size() >= 1) {
			std::string type_str;
			for (auto i = 0; i != args[0].size(); ++i) {
				type_str += toupper(args[0][i]);
			}
			type = getEntityTypeFromString(type_str);
		}
		if (args.size() >= 3) { pos = vec2(float(atof(args[1].c_str())), float(atof(args[2].c_str()))); }
		//if (args.size() == 2) { flags = getFlagsFromString(args[1]); }
		//if (args.size() == 4) { flags = getFlagsFromString(args[3]); }

		impl->editor->world.AddEntity(make_entity(type, pos));
	});
	AddInfo("NewEnt", "Create a new entity",new_ent_usage);

	add_cmd("del", {
		if (args.size() == 1) {
			impl->editor->world.DeleteEntity(atoi(args[0].c_str()));
		}
		else {
			impl->console->AddLine("Usage:\ndel id", ERROR);
		}
	});
	AddInfo("del", "Delete an entity", "Usage:\ndel id");

	add_cmd("clear", {
		impl->console->ClearLines();
	});
	AddInfo("clear", "Clear the console", "Just write clear. No arguments needed.");

	add_cmd("controls", {
		impl->console->AddLine("\
LCtrl + scroll                 Zoom\n\
LeftMouseButton drag           Move entity under mouse\n\
MidMouseButton drag            Move view\n\
MidMouseButton press           Print info of entity under mouse in console\n\
F                              Fill under mouse with tile type (when ground edit mode)\n\
D                              Duplicate entity under mouse\n\
Del                            Delete entity under mouse"
, RESULT);
	});
	AddInfo("controls", "Display keyboard + mouse controls", "Just write controls.");

	add_cmd("help", {
		if (!args.size()) {
			for (auto c : impl->console->getCommands()) {
				int nb_of_spaces = 30 - c->name.size();
				string str = c->name;
				while (nb_of_spaces--) str+=' ';
				str += c->desc;
				impl->console->AddLine(str, RESULT);
			}
		}
		else if (args.size() == 1) {
			std::string name = args[0];
			auto c = impl->console->getCommand(name);
			if (c)	impl->console->AddLine(c->help_string					 , RESULT);
			else	impl->console->AddLine("\"" + name + "\" is not a command", ERROR);
		}
		else {
			impl->console->AddLine("Too many arguments given. Expected 0 or 1", ERROR);
		}
	});
	AddInfo("help", "Display available commands or help for specific command",
			"Usage:\nhelp            List commands\nhelp command    Display help for command"
	);

	add_cmd("quit", {
		impl->editor->Quit();
	});
	AddInfo("quit", "Quit editor", "Just write quit. No arguments needed.");

	#undef add_cmd
	#undef new_ent_usage
}

