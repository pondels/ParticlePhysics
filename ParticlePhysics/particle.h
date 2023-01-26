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
};

class Fire : public Particle {
public: 
	std::string type = "fire";
	float temp = 72.f; // Default Room Temp

	using Particle::Particle;
};

class Water : public Particle {
public:
	std::string type = "water";
	float viscocity = 1; // Default water viscosity

	using Particle::Particle;
};

#endif