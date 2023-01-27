#pragma once
#include <SFML/Graphics.hpp>

#ifndef PARTICLE_H
#define PARTICLE_H

class Particle {
public:

	sf::CircleShape* particle;

	std::string type;
	float radius;
	double mass;
	double vy;
	float temperature;
	sf::Vector2f* velocity;
	int viscosity;

	Particle(float, sf::Vector2f, sf::Color, std::string, double, sf::Vector2f*, float, int);
};

#endif