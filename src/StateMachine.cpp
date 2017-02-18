#include "StateMachine.h"

void StateMachine::PushState(State state)
{
	state_stack.push_back(state);
}

State StateMachine::PopState()
{
	State s = getActiveState();
	if (state_stack.size() > 0) {
		state_stack.pop_back();
	}
	return s;
}
