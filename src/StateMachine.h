#pragma once

#include <vector>

enum class State
{
	MainMenu,
	InfoMenu,
	OptionsMenu,
	AudioOptionsMenu,
	PausedMenu,
	Game
};

class StateMachine
{
public:
	void PushState(State state); // when entering a new state
	State PopState(); // when returning to previous state

	State getLastState() { return state_stack[state_stack.size() - 2]; }
	State getActiveState() { return state_stack[state_stack.size() - 1]; }

private:
	std::vector<State> state_stack; // keeps track of active states
};
