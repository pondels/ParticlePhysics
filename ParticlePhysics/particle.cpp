#include <iostream>
#include "particle.h"

Particle::Particle(float size, sf::Vector2f position, sf::Color color, std::string particle_type, double particle_mass, sf::Vector2f* v, float temp, int visc) {
	particle = new sf::CircleShape(size);
	particle->setPosition(position);
	particle->setOrigin(particle->getGlobalBounds().width / 2, particle->getGlobalBounds().height / 2);
	particle->setFillColor(color);

	static const int DIM = 2;
	mass = particle_mass;
	velocity = v;
	radius = size;
	type = particle_type;
	temperature = temp;
	viscosity = visc;
}