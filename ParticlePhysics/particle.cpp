#include <iostream>
#include "particle.h"

Particle::Particle(float size, sf::Vector2f position, sf::Color color, int phys_type, double particle_mass, sf::Vector2f v) {
	particle = new sf::CircleShape(size);
	particle->setPosition(position);
	particle->setOrigin(particle->getGlobalBounds().width / 2, particle->getGlobalBounds().height / 2);
	particle->setFillColor(color);

	//double start_time = clock();
	mass = particle_mass;
	velocity = v;

	type = gravity_type(phys_type);
}

std::string Particle::gravity_type(int phys_type) {
	switch (phys_type) {
		case 0: {
			return "Space";
		}
		case 1: {
			return "Normal";
		}
	}
}