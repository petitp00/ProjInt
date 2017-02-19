#pragma once

#include <string>
#include <map>

#include <SFML/Graphics.hpp>

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics/Shader.hpp>

class ResourceManager
{
public:

	// Fonts
	static sf::Font& getFont(std::string name);
	static std::map<std::string, sf::Font> fonts;

	// Textures
	static sf::Texture& getTexture(std::string name);
	static std::map<std::string, sf::Texture> textures;
};