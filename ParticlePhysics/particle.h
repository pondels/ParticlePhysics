#pragma once
#include <SFML/Graphics.hpp>

#ifndef PARTICLE_H
#define PARTICLE_H

class Particle {
public:

	std::unique_ptr<sf::CircleShape> particle;
	std::string type;
	float radius;
	double mass;
	double vy;
	float temperature;
	std::unique_ptr<sf::Vector2f> velocity;
	float viscosity;
	bool consume;
	bool explode;
	bool teleportation;
	bool swap;
	bool iridescent;
	bool radioactive;

	Particle() = default;
	Particle(float size, sf::Vector2f position, sf::Color color, std::string particle_type, double particle_mass, sf::Vector2f v, float temp, float visc, bool cons, bool xplod, bool teleport, bool particle_swap, bool iridescence, bool is_radioactive) {
		particle = std::make_unique<sf::CircleShape>(size);
		particle->setPosition(position);
		particle->setOrigin(size, size);
		particle->setFillColor(color);

		mass = particle_mass;
		velocity = std::make_unique<sf::Vector2f>(v.x, v.y);
		radius = size;
		type = particle_type;
		temperature = temp;
		viscosity = visc;
		consume = cons;
		explode = xplod;
		teleportation = teleport;
		swap = particle_swap;
		iridescent = iridescence;
		radioactive = is_radioactive;
	}
};

#endif