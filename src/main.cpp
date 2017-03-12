#include "ResourceManager.h"

#ifndef EDITOR_MODE
#include "Game.h"
#endif
#ifdef EDITOR_MODE
#include "Editor/Editor.h"
#endif

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
		EditorMode::Editor editor;
		editor.Start();
#endif

	cout << "Deleting resources ..." << endl;
	ResourceManager::ClearAll();
	cout << "Done!" << endl;

	return 0;
}
