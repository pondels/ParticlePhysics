#pragma once
#include <SFML/Graphics.hpp>

#ifndef UI_H
#define UI_H

class UserInterface {
public:
	
	sf::Vector2f windowsize;
	std::vector<std::vector<sf::RectangleShape>> vectors;
	std::vector<sf::Text*> texts;
	sf::Font font;

	UserInterface(sf::Vector2f);
	float convert_x(float);
	float convert_y(float);
	sf::Vector2f convert_resolution(sf::Vector2f);
	sf::RectangleShape make_bar(sf::Vector2f, sf::Color, sf::Vector2f);
	void create_UI(int, int, int, int, int, int, int, int, int, float, int);
	void PictureDisplay();
	void TextDisplay(int, int, int, int, int, int, int, int, int, float, int);
};

#endif