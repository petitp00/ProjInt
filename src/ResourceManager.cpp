#include "ResourceManager.h"

#include <iostream>
using namespace std;

map<string, sf::Font> ResourceManager::fonts;

sf::Font& ResourceManager::getFont(std::string name)
{
	if (fonts.count(name)) return fonts[name];

	string path = "Resources/Fonts/";

	sf::Font f;
	if (!f.loadFromFile(path + name)) {
		cerr << "Couldn't load font \"" << name << "\"." << endl;
	}

	fonts[name] = f;
	return fonts[name];
}
