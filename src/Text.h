#pragma once

#include <SFML/Graphics/Text.hpp>

/*
	A text container with word wrapping 
*/
class TextBox
{
public:
	TextBox() = default;
	TextBox(
		std::string text_string,
		sf::Vector2f pos,
		sf::Vector2f size,
		sf::Font* font,
		sf::Color color,
		unsigned int character_size
	);

	TextBox(
		std::string text_string,
		sf::Vector2f pos,
		sf::Vector2f size,
		std::string const& font_name,
		sf::Color color,
		unsigned int character_size
	);

	void Render(sf::RenderTarget& target);

	void setPos(sf::Vector2f pos, bool update=true);
	void setSize(sf::Vector2f size, bool update=true);
	void setFont(sf::Font* font, bool update=true);
	void setFont(std::string const& font_name, bool update=true);
	void setColor(sf::Color color, bool update=true);
	void setCharacterSize(unsigned int character_size, bool update=true);
	void setTextString(std::string const& text_string, bool update=true);

	sf::Vector2f getPos() { return pos; }
	sf::Vector2f getSize() { return size; }
	sf::Font* getFont() { return font; }
	sf::Color getColor() { return color; }
	unsigned int getCharacterSize() { return character_size; }
	std::string getTextString() { return text_string; }

private:
	void UpdateTextBox(bool set_text_obj_params = true, int start_at_index = 0);

	sf::Vector2f pos;
	sf::Vector2f size;
	sf::Font* font;
	sf::Color color;
	unsigned int character_size;

	std::string text_string;
	std::string original_text_string;
	
	sf::Text text_obj;
};
