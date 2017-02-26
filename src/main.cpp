#include "Game.h"
#include "ResourceManager.h"

#include <iostream>
using namespace std;


int main()
{
	srand(100);

	{
		Game game;

		game.Start();
	}

	cout << "Deleting resources ..." << endl;
	ResourceManager::ClearAll();
	cout << "Done!" << endl;

	return 0;
}
