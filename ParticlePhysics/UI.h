#pragma once
#include <SFML/Graphics.hpp>

#ifndef UI_H
#define UI_H

class UserInterface {
public:
	
	sf::Vector2f windowsize;
	std::vector<std::vector<sf::RectangleShape*>> vectors;
	std::vector<sf::Text*> texts;
	std::vector<sf::CircleShape*> preview_particles;
	sf::Font font;

	UserInterface(sf::Vector2f);
	float convert_x(float);
	float convert_y(float);
	sf::Vector2f convert_resolution(sf::Vector2f);
	sf::RectangleShape* make_bar(sf::Vector2f, sf::Color, sf::Vector2f);
	void move_UI(sf::Vector2i);
	void check_collision(std::string&, int&, sf::Vector2i, int&, int&, int&, int&, int&,
		int&, int&, int&, int&, int&, bool&, float&, int&, int&, float&, bool&, bool&, bool&,
		bool&, bool&, bool&, bool&, bool&, bool&, bool&, bool&,
		std::string&);
	void create_UI(int, int, int, int, int, int, int, int, int, float, int, float, float);
	void PictureDisplay();
	void TextDisplay(int, int, int, int, int, int, int, int, int, float, int, int, int);
};

#endif