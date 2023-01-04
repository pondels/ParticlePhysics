#include <iostream>
#include <SFML/Graphics.hpp>
#include "particle.h"

void DeltaTime(double& lastTime, double& currentTime, double& deltaTime) {
    deltaTime = (currentTime - lastTime) / 1000.f;
    lastTime = currentTime;
}
void check_collisions(std::vector<Particle*> particles, sf::CircleShape* particle) {
    
    // Update Direction & Speed of particle based on collisions.
    return;
}
void update_position(Particle* particle, sf::CircleShape* shape, int index, double deltaTime) {
    
    // Move particle so far along the given path.
    float x = shape->getPosition().x;
    float y = shape->getPosition().y;
    particle->velocity.x += .1;
    particle->velocity.y += .1;
    shape->setPosition(x + particle->velocity.x * particle->mass * deltaTime, y + particle->velocity.y * particle->mass * deltaTime);
}
void update_particle(std::vector<Particle*> particles, int index, double deltaTime) {
    for (int i = 0; i < particles.size(); i++) {

        // Particle can't collide with itself
        if (i != index) {
            // Check for Collisions
            check_collisions(particles, particles.at(index)->particle);
        }
    }
    update_position(particles.at(index), particles.at(index)->particle, index, deltaTime);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
 
    std::vector<Particle*> particles;

    int particle_amount = 6;

    for (int i = 0; i < particle_amount; i++) {
        float size = 3.f;
        sf::Vector2f position(10 + 10 * i, 10 + 10 * i);
        sf::Color color(0, 0, 255);
        int grav_type = 1;
        double mass = 1.5;
        sf::Vector2f velocity(0, 9.81);

        Particle* particle = new Particle(size, position, color, grav_type, mass, velocity);
        particles.push_back(particle);
    }

    double lastTime = clock();
    double deltaTime;

    while (window.isOpen())
    {
        double currentTime = clock();
        DeltaTime(lastTime, currentTime, deltaTime);

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();

        // Draws Pixels
        for (int i = 0; i < particles.size(); i++) {
            window.draw(*particles.at(i)->particle);
        }
        
        // Applies Updates to pixels after drawing
        for (int i = 0; i < particles.size(); i++) {
            update_particle(particles, i, deltaTime);
        }

        window.display();
    }

    return 0;
}