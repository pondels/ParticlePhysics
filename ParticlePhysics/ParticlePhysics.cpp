#include <iostream>
#include <SFML/Graphics.hpp>
#include "particle.h"

void DeltaTime(double& lastTime, double& currentTime, double& deltaTime) {
    deltaTime = (currentTime - lastTime) / 1000.f;
    lastTime = currentTime;
}
bool check_collisions(std::vector<Particle*> particles, Particle* particle, sf::CircleShape* shape) {
    
    // Update Direction & Speed of particle based on collisions.

    sf::Vector2f windowsize(1280, 720);

    // Colliding with the ground
    if (shape->getPosition().y + (shape->getGlobalBounds().height / 2) >= windowsize.y) {
        particle->velocity.y = -particle->velocity.y;
    }
    else if (shape->getPosition().y - (shape->getGlobalBounds().height / 2) <= 0 && particle->velocity.y < 0) {
        particle->velocity.y = -particle->velocity.y;
    }
    /*else if (shape->getPosition().x - (shape->getGlobalBounds().height / 2) <= windowsize.y) {
        return false;
    }
    else if (shape->getPosition().y - (shape->getGlobalBounds().height / 2) <= windowsize.y) {
        return false;
    }*/
    return true;
}
void update_position(Particle* particle, sf::CircleShape* shape, int index, double deltaTime) {
    
    // Move particle so far along the given path.
    float gravity = 9.81 / 5;
    float x = shape->getPosition().x;
    float y = shape->getPosition().y;
    //particle->velocity.x += .1;
    shape->setPosition(x + particle->velocity.x, y + particle->velocity.y);
    particle->velocity.y += particle->mass * deltaTime * gravity;
}
void update_particle(std::vector<Particle*> particles, int index, double deltaTime) {

    check_collisions(particles, particles.at(index), particles.at(index)->particle);
    update_position(particles.at(index), particles.at(index)->particle, index, deltaTime);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML works!");
 
    std::vector<Particle*> particles;

    int particle_amount = 6;

    for (int i = 0; i < particle_amount; i++) {
        float size = 10.f;
        sf::Vector2f position(10 + 10 * i, 10 + 10 * i);
        sf::Color color(0, 0, 255);
        int grav_type = 1;
        double mass = .5;
        sf::Vector2f velocity(0, 0);

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