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
	static void ClearAll();

	// Fonts
	static sf::Font& getFont(std::string name);
	static std::map<std::string, sf::Font> fonts;

	// Textures
	static sf::Texture& getTexture(std::string name);
	static std::map<std::string, sf::Texture> textures;
};

struct Sound {
	sf::SoundBuffer buf;
	sf::Sound sound;
	float relative_volume = 0;
};

class SoundManager
{
public:
	static void Clear();
	static Sound* getSound(std::string name);
	static void Play(std::string name);

	static void setMute(bool mute);
	static void setVolume(float volume);
	static void setRelativeVolume(std::string name, float relative_volume);

	static std::map<std::string, Sound*> sounds;
	static bool mute;
	static float volume;
};
