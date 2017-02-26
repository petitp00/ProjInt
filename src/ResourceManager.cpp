#include "ResourceManager.h"

#include <iostream>
using namespace std;

map<string, sf::Font> ResourceManager::fonts;

void ResourceManager::ClearAll()
{
	ResourceManager::fonts.clear();
	ResourceManager::textures.clear();
}

sf::Font& ResourceManager::getFont(std::string name)
{
	if (fonts.count(name)) return fonts[name];

	string path = "Resources/Fonts/";

	sf::Font f;
	if (!f.loadFromFile(path + name)) {
		cerr << "Could not load font \"" << name << "\"." << endl;
	}

	fonts[name] = f;
	return fonts[name];
}

map<string, sf::Texture> ResourceManager::textures;

sf::Texture& ResourceManager::getTexture(std::string name)
{
	if (textures.count(name)) return textures[name];

	string path = "Resources/Textures/";

	sf::Texture t;
	if (!t.loadFromFile(path + name)) {
		cerr << "Could not load texture \"" << name << "\"." << endl;
	}

	textures[name] = t;
	return textures[name];
}
