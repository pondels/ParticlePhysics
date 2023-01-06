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

void check_collisions(std::vector<Particle*> particles, Particle* particle, sf::CircleShape* shape, int index) {
    
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
    if (particlex < radius) {
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
            float y1 = particles.at(i)->particle->getPosition().y;
            float r1 = particles.at(i)->particle->getGlobalBounds().width / 2;
            double m1 = particles.at(i)->mass;
            sf::Vector2f* v1 = particles.at(i)->velocity;
            
            float x2 = shape->getPosition().x;
            float y2 = shape->getPosition().y;
            float r2 = shape->getGlobalBounds().width / 2;
            double m2 = particle->mass;
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
                if (speed < 0) {
                    break;
                }

                float lapover = (distance - (particles.at(i)->particle->getGlobalBounds().width * 0.5) - (particle->particle->getGlobalBounds().width * 0.5)) / 2.f;

                // If inside an orb, break out
                float moveX = (lapover * (x1 - x2) / distance);
                float moveY = (lapover * (y1 - y2) / distance);

                particles.at(i)->particle->setPosition(x1 - moveX, y1 - moveY);
                particle->particle->setPosition(x2 + moveX, y2 + moveY);

                speed *= restitution;
                float impulse = 2 * speed / (m1 + m2);

                v1->x -= impulse * m2 * vCollisionNorm.x;
                v1->y -= impulse * m2 * vCollisionNorm.y;
                v2->x += impulse * m1 * vCollisionNorm.x;
                v2->y += impulse * m1 * vCollisionNorm.y;
            }
        }
    }

    if (!overlap) {
        shape->setFillColor(sf::Color(0, 0, 255));
    }
}

void update_position(Particle* particle, sf::CircleShape* shape, int index, double deltaTime) {
    
    // Move particle so far along the given path.
    float gravity = 9.81;
    float x = shape->getPosition().x;
    float y = shape->getPosition().y;

    // Applying gravity to particle
    particle->velocity->y += deltaTime * gravity * particle->mass;

    shape->setPosition(x + particle->velocity->x * deltaTime, y + particle->velocity->y * deltaTime);
}
void update_particle(std::vector<Particle*> particles, int index, double deltaTime) {

    check_collisions(particles, particles.at(index), particles.at(index)->particle, index);
    update_position(particles.at(index), particles.at(index)->particle, index, deltaTime);
}
int main()
{
    sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML works!");
 
    std::vector<Particle*> particles;

    // Standard particle features
    float size = 5.f;
    double mass = 150.f;

    double lastTime = clock();
    double deltaTime;
    int particle_amount = 1;

    while (window.isOpen())
    {
        double currentTime = clock();
        DeltaTime(lastTime, currentTime, deltaTime);

        sf::Event event;
        while (window.pollEvent(event))
        {

            if (event.type == sf::Event::Closed) window.close();
            else if (event.type == sf::Event::KeyPressed)
            {
                int mouseX = sf::Mouse::getPosition().x;
                int mouseY = sf::Mouse::getPosition().y;

                if (event.key.code == sf::Keyboard::Add)
                {
                    std::cout << particles.size() << std::endl;
                    for (int i = 0; i < particles.size(); i++) {
                        particles.at(i)->mass += 5.f;
                        particles.at(i)->particle->setRadius(particles.at(i)->particle->getRadius() + 1.f);
                    }
                    size += 1.f;
                    mass += 5.f;
                }
                else if (event.key.code == sf::Keyboard::Subtract) {
                    for (int i = 0; i < particles.size(); i++) {
                        particles.at(i)->mass -= 5.f;
                        particles.at(i)->particle->setRadius(particles.at(i)->particle->getRadius() - 1.f);
                    }
                    size -= 1.f;
                    mass -= 5.f;
                }
                else if (event.key.code == sf::Keyboard::Delete) {
                    particles.clear();
                }
                else if (event.key.code == sf::Keyboard::Up) {
                    particle_amount += 1;
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    particle_amount -= 1;
                }
                else {
                    for (int i = 0; i < particle_amount; i++) {
                        sf::Vector2f position(mouseX+30*i, mouseY);
                        sf::Color color(0, 0, 255);
                        int grav_type = 1;
                        sf::Vector2f* velocity = new sf::Vector2f(0, 0);

                        Particle* particle = new Particle(size, position, color, grav_type, mass, velocity);
                        particles.push_back(particle);
                    }
                }
            }
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