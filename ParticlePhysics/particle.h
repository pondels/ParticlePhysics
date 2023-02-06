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

	Particle() = default;
	Particle(float size, sf::Vector2f position, sf::Color color, std::string particle_type, double particle_mass, sf::Vector2f v, float temp, int visc) {
		particle = new sf::CircleShape(size);
		particle->setPosition(position);
		particle->setOrigin(particle->getGlobalBounds().width / 2, particle->getGlobalBounds().height / 2);
		particle->setFillColor(color);

		static const int DIM = 2;
		mass = particle_mass;
		velocity = new sf::Vector2f(v.x, v.y);
		radius = size;
		type = particle_type;
		temperature = temp;
		viscosity = visc;
	}
	~Particle() {
		delete particle;
		delete velocity;
	}
};

#endif