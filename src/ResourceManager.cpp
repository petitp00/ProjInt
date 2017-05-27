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

// SOUND MANAGER //
map<string, Sound*> SoundManager::sounds;
bool SoundManager::mute = false;
float SoundManager::volume = 100.f;

void SoundManager::Clear()
{
	SoundManager::sounds.clear();
}

Sound* SoundManager::getSound(std::string name)
{
	if (sounds.count(name)) {
		return sounds[name];
	}

	string path = "Resources/Sounds/";
	Sound* s = new Sound;
	if (!s->buf.loadFromFile(path + name)) {
		cerr << "Could not load sound buffer \"" << name << "\"." << endl;
	}
	s->sound.setBuffer(s->buf);
	s->relative_volume = 0.f;

	float vol = volume + volume * s->relative_volume;
	s->sound.setVolume(max(0.f, min(100.f, vol)));

	sounds[name] = s;
	return sounds[name];
}

void SoundManager::Play(std::string name)
{
	if (!mute) {
		getSound(name)->sound.play();
	}
}

void SoundManager::setMute(bool mute)
{
	SoundManager::mute = mute;
}

void SoundManager::setVolume(float volume)
{
	SoundManager::volume = volume;
	for (auto s : sounds) {
		float vol = volume + volume * s.second->relative_volume;
		s.second->sound.setVolume(max(0.f, min(100.f, vol)));
	}
}

void SoundManager::setRelativeVolume(std::string name, float relative_volume)
{
	auto s = getSound(name);
	s->relative_volume = relative_volume;
	float vol = volume + volume * relative_volume;
	s->sound.setVolume(max(0.f, min(100.f, vol)));
}
