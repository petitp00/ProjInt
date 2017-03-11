#include "Game.h"
#include "ResourceManager.h"

#include <iostream>
using namespace std;


int main()
{
	srand(100);

#ifndef EDITOR_MODE
		Game game;

		game.Start();
#endif

#ifdef EDITOR_MODE

#endif

	cout << "Deleting resources ..." << endl;
	ResourceManager::ClearAll();
	cout << "Done!" << endl;

	return 0;
}
