#pragma once
#include <SFML/Graphics.hpp>
#include "physics.h"

#ifndef PARTICLE_H
#define PARTICLE_H

class Particle {
public:

	sf::CircleShape* particle;
	std::string type;
	float radius;
	double mass;
	double vy;
	sf::Vector2f* velocity;

	Particle(float, sf::Vector2f, sf::Color, int, double, sf::Vector2f*);
	std::string gravity_type(int);
};

#endif