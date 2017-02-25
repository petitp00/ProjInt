#include "Entity.h"
#include "ResourceManager.h"

using namespace std;

GameObject::GameObject(sf::Vector2f pos, std::string texture_name, sf::Vector2f size, unsigned long flags) :
	Entity(pos, size, flags)
{
	sprite.setTexture(ResourceManager::getTexture(texture_name));
	sprite.setPosition(pos);
	
	auto b = sprite.getLocalBounds();

	if (size == sf::Vector2f{ 0, 0 }) { size = { b.width, b.height }; }
	else { sprite.setScale(size.x / b.width, size.y / b.height); }
}
