#include <iostream>
#include <SFML/Graphics.hpp>
#include "particle.h"
#include <cmath>

/*

*****SOURCES*****
https://spicyyoghurt.com/tutorials/html5-javascript-game-development/collision-detection-physics

*/

void DeltaTime(double& lastTime, double& currentTime, double& deltaTime) {
    deltaTime = (currentTime - lastTime) / 1000.f;
    lastTime = currentTime;
}
bool check_collisions(std::vector<Particle*> particles, Particle* particle, sf::CircleShape* shape, int index) {
    
    // Update Direction & Speed of particle based on collisions.

    // Rate at which energy is lost
    float restitution = .9f;

    sf::Vector2f windowsize(1280, 720);

    float radius = shape->getGlobalBounds().height / 2;
    float particlex = shape->getPosition().x;
    float particley = shape->getPosition().y;

    // Colliding with the ground
    if (particley > windowsize.y - radius) {
        particle->velocity->y = -abs(particle->velocity->y) * restitution;
        particle->particle->setPosition(particlex, windowsize.y - radius);
    }

    // Colliding with the ceiling
    else if (particley < radius) {
        particle->velocity->y = abs(particle->velocity->y) * restitution;
        particle->particle->setPosition(particlex, radius);
    }

    // Colliding with the walls
    else if (particlex < radius) {
        particle->velocity->x = abs(particle->velocity->x) * restitution;
        particle->particle->setPosition(radius, particley);
    }
    else if (particlex > windowsize.x - radius) {
        particle->velocity->x = -abs(particle->velocity->x) * restitution;
        particle->particle->setPosition(windowsize.x - radius, particley);
    }

    bool overlap = false;

    // Colliding with other particles
    for (int i = 0; i < particles.size(); i++) {
        if (i != index) {
            float x1 = particles.at(i)->particle->getPosition().x;
            float x2 = shape->getPosition().x;
            float y1 = particles.at(i)->particle->getPosition().y;
            float y2 = shape->getPosition().y;
            float r1 = particles.at(i)->particle->getGlobalBounds().width / 2;
            float r2 = shape->getGlobalBounds().width / 2;
            double m1 = particles.at(i)->mass;
            double m2 = particle->mass;
            sf::Vector2f* v1 = particles.at(i)->velocity;
            sf::Vector2f* v2 = particle->velocity;
            
            float squaredistance = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
            if (squaredistance <= ((r1 + r2) * (r1 + r2))) {
                shape->setFillColor(sf::Color(255, 0, 0));
                overlap = true;

                sf::Vector2f vCollision(x2 - x1, y2 - y1);
                float distance = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
                sf::Vector2f vCollisionNorm(vCollision.x / distance, vCollision.y / distance);
                sf::Vector2f vRelativeVelocity(v1->x - v2->x, v1->y - v2->y);
                float speed = vRelativeVelocity.x * vCollisionNorm.x + vRelativeVelocity.y * vCollisionNorm.y;
                speed *= restitution;
                float impulse = 2 * speed / (m1 + m2);

                particles.at(index)->velocity->x = impulse * m2 * vCollisionNorm.x;
                particles.at(index)->velocity->y = impulse * m2 * vCollisionNorm.y;
                particle->velocity->x = impulse * m1 * vCollisionNorm.x;
                particle->velocity->y = impulse * m1 * vCollisionNorm.y;
                break;
            }
        }
    }

    if (!overlap) {
        shape->setFillColor(sf::Color(0, 0, 255));
    }

    return true;
}
void update_position(Particle* particle, sf::CircleShape* shape, int index, double deltaTime) {
    
    // Move particle so far along the given path.
    float gravity = 9.81 / 5;
    float x = shape->getPosition().x;
    float y = shape->getPosition().y;
    shape->setPosition(x + particle->velocity->x, y + particle->velocity->y);
    
    // Applying gravity to particle
    particle->velocity->y += particle->mass * deltaTime * gravity;
}
void update_particle(std::vector<Particle*> particles, int index, double deltaTime) {

    check_collisions(particles, particles.at(index), particles.at(index)->particle, index);
    update_position(particles.at(index), particles.at(index)->particle, index, deltaTime);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML works!");
 
    std::vector<Particle*> particles;

    int particle_amount = 10;

    for (int i = 0; i < particle_amount; i++) {
        float size = 15.f;
        sf::Vector2f position(10 + 45 * i, 50);
        sf::Color color(0, 0, 255);
        int grav_type = 1;
        double mass = .1 + .1*i;
        sf::Vector2f* velocity = new sf::Vector2f(.1, .1);

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