#include <iostream>
#include <SFML/Graphics.hpp>
#include "particle.h"
#include <cmath>
#include "KDTree.h"

/*

*****SOURCES*****
https://spicyyoghurt.com/tutorials/html5-javascript-game-development/collision-detection-physics
https://chat.openai.com/chat
*/

sf::Vector2f windowsize(1920, 1080);
void UI() {
     
}
float dot(sf::Vector2f a, sf::Vector2f b) {
    return a.x * b.x + a.y * b.y;
}
bool vertical_overlap(float y1, float y2, float r1, float r2) {
    if (abs(y1 - y2) - (r1 + r2) < 0) return true;
    return false;
}
bool horizontal_overlap(float x1, float x2, float r1, float r2) {
    if (abs(x1 - x2) - (r1 + r2) < 0) return true;
    return false;
}
void check_collisions1(std::vector<Particle*> particles, Particle* particle, sf::CircleShape* shape, int index) {

    // Update Direction & Speed of particle based on collisions.

    // Rate at which energy is lost against the wall/floor
    float restitution = .9f;

    float radius = shape->getGlobalBounds().height / 2;
    float particlex = shape->getPosition().x;
    float particley = shape->getPosition().y;

    // Colliding with the walls
    if (particlex < radius) {
        particle->velocity->x *= -restitution;
        particle->particle->setPosition(radius, particley);
        particlex = radius;
    }
    else if (particlex > windowsize.x - radius) {
        particle->velocity->x *= -restitution;
        particle->particle->setPosition(windowsize.x - radius, particley);
        particlex = windowsize.x - radius;
    }

    // Colliding with the ground
    if (particley > windowsize.y - radius) {
        particle->velocity->y *= -restitution;
        particle->particle->setPosition(particlex, windowsize.y - radius);
    }

    // Colliding with the ceiling
    else if (particley < radius) {
        particle->velocity->y *= -restitution;
        particle->particle->setPosition(particlex, radius);
    }

    // Colliding with other particles
    for (int i = 0; i < particles.size(); i++) {
        if (i != index) {
            float x1 = particles[i]->particle->getPosition().x;
            float y1 = particles[i]->particle->getPosition().y;
            float r1 = particles[i]->radius;
            double m1 = particles[i]->mass;
            sf::Vector2f* v1 = particles[i]->velocity;

            float x2 = shape->getPosition().x;
            float y2 = shape->getPosition().y;
            float r2 = particle->radius;
            double m2 = particle->mass;
            sf::Vector2f* v2 = particle->velocity;

            bool ho = horizontal_overlap(x1, x2, r1, r2);
            if (ho) {
                bool vo = vertical_overlap(y1, y2, r1, r2);
                if (vo) {
                    float squaredistance = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
                    if (squaredistance < ((r1 + r2) * (r1 + r2)) && squaredistance != 0) {
                        float distance = sqrtf(squaredistance);
                        float overlap = (distance - r1 - r2) / 2.f;

                        // If inside a particle, break out
                        float moveX = (overlap * (x1 - x2) / distance);
                        float moveY = (overlap * (y1 - y2) / distance);

                        particles[i]->particle->setPosition(x1 - moveX, y1 - moveY);
                        particle->particle->setPosition(x2 + moveX, y2 + moveY);

                        sf::Vector2f vCollision(x2 - x1, y2 - y1);
                        sf::Vector2f vCollisionNorm(vCollision.x / distance, vCollision.y / distance);
                        sf::Vector2f vRelativeVelocity(v1->x - v2->x, v1->y - v2->y);
                        float speed = vRelativeVelocity.x * vCollisionNorm.x + vRelativeVelocity.y * vCollisionNorm.y;
                        if (speed < 0) {
                            break;
                        }

                        speed *= restitution;
                        float impulse = float((2 * speed) / (m1 + m2));

                        v1->x -= speed * vCollisionNorm.x;
                        v1->y -= speed * vCollisionNorm.y;
                        v2->x += speed * vCollisionNorm.x;
                        v2->y += speed * vCollisionNorm.y;
                    }
                }
            }
        }
    }
}
void check_collisions2(std::vector<Particle*> particles, Particle* particle, sf::CircleShape* shape, int index) {
    
    // Update Direction & Speed of particle based on collisions.

    // Rate at which energy is lost against the wall/floor
    float restitution = .9f;

    float radius = shape->getGlobalBounds().height / 2;
    float particlex = shape->getPosition().x;
    float particley = shape->getPosition().y;

    // Colliding with the walls
    if (particlex < radius) {
        particle->velocity->x *= -restitution;
        particle->particle->setPosition(radius, particley);
        particlex = radius;
    }
    else if (particlex > windowsize.x - radius) {
        particle->velocity->x *= -restitution;
        particle->particle->setPosition(windowsize.x - radius, particley);
        particlex = windowsize.x - radius;
    }

    // Colliding with the ground
    if (particley > windowsize.y - radius) {
        particle->velocity->y *= -restitution;
        particle->particle->setPosition(particlex, windowsize.y - radius);
    }

    // Colliding with the ceiling
    else if (particley < radius) {
        particle->velocity->y *= -restitution;
        particle->particle->setPosition(particlex, radius);
    }

    for (int i = 0; i < particles.size(); i++) {
        if (i != index) {
            float x1 = particles[i]->particle->getPosition().x;
            float y1 = particles[i]->particle->getPosition().y;
            float r1 = particles[i]->radius;
            double m1 = particles[i]->mass;
            sf::Vector2f* v1 = particles[i]->velocity;

            float x2 = shape->getPosition().x;
            float y2 = shape->getPosition().y;
            float r2 = particle->radius;
            double m2 = particle->mass;
            sf::Vector2f* v2 = particle->velocity;

            // Check for collision
            float distance = sqrtf((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
            if (distance <= r1 + r2 && distance != 0) {
                // Calculate the collision normal
                float overlap = (distance - r1 - r2) / 2.f;

                // If inside a particle, break out
                float moveX = (overlap * (x1 - x2) / distance);
                float moveY = (overlap * (y1 - y2) / distance);

                particles[i]->particle->setPosition(x1 - moveX, y1 - moveY);
                particle->particle->setPosition(x2 + moveX, y2 + moveY);

                // Calculate the impulse scalar
                sf::Vector2f normal((x2 - x1) / distance, (y2 - y1) / distance);
                sf::Vector2f tangent(-normal.y, normal.x);
                sf::Vector2f relativeVelocity = *v2 - *v1;

                float impulseScalar = dot(relativeVelocity, tangent) * restitution / (1.0f / m1 + 1.0f / m2);
                sf::Vector2f impulse = impulseScalar * tangent;
                //float dotProductTangent1 = v1->x * normal.x + v1->y * normal.y;
                //float dotProductTangent2 = v2->x * normal.x + v2->y * normal.y;

                //v1->x = normal.x * dotProductTangent1;
                //v1->y = normal.y * dotProductTangent1;
                //v2->x = normal.x * dotProductTangent2;
                //v2->y = normal.y * dotProductTangent2;
                v1->x += impulse.x / m1;
                v1->y += impulse.y / m1;
                v2->x -= impulse.x / m2;
                v2->y -= impulse.y / m2;
            }
        }
    }
}
void update_position(Particle* particle, sf::CircleShape* shape, int index, float deltaTime) {
    
    // Move particle so far along the given path.
    float gravity = 9.81f;
    float x = shape->getPosition().x;
    float y = shape->getPosition().y;

    // Applying gravity to particle
    particle->velocity->y += deltaTime * gravity * particle->mass;

    shape->setPosition(x + particle->velocity->x * deltaTime, y + particle->velocity->y * deltaTime);
}
void update_particle(std::vector<Particle*> particles, int index, float deltaTime) {
    update_position(particles[index], particles[index]->particle, index, deltaTime);
    check_collisions2(particles, particles[index], particles[index]->particle, index);
}
sf::Color color_getter(int &r, int &g, int &b, bool &r_dir, bool &g_dir, bool &b_dir) {

    // How much the color gap is between particles
    int gap = 15;

    if (r < 255 && r_dir) {
        g_dir = true;
        for (int j = 0; j < gap; j++) r++;
    }
    else if (b > 0 && !b_dir) {
        for (int j = 0; j < gap; j++) b--;
    }
    else if (g < 255 && g_dir) {
        r_dir = false;
        for (int j = 0; j < gap; j++) g++;
    }
    else if (g >= 255 && r_dir == false && r > 0) {
        b_dir = true;
        g_dir = false;
        for (int j = 0; j < gap; j++) r--;
    }
    else if (r <= 0 && b < 255 && b_dir) {
        for (int j = 0; j < gap; j++) b++;
    }
    else if (b >= 255 && !g_dir && g > 0) {
        for (int j = 0; j < gap; j++) g--;
    }
    else {
        b_dir = false;
        r_dir = true;
    }

    sf::Color color(r, g, b);
    return color;
}
int main()
{
    int fps = 165;
    sf::RenderWindow window(sf::VideoMode(windowsize.x, windowsize.y), "SFML works!", sf::Style::Fullscreen);
    window.setFramerateLimit(fps);
 
    std::vector<Particle*> particles;

    std::vector<sf::Vector3f> positions;
    for (int i = 0; i < particles.size(); i++) {
        positions.push_back(sf::Vector3f(particles[i]->particle->getPosition().x, particles[i]->particle->getPosition().y, particles[i]->radius));
    }

    //kdt::KDTree<sf::Vector3f> kdtree(positions);

    // Standard particle features
    float size = 5.f;
    double mass = 150.f;

    float deltaTime = 1.f/fps;
    int particle_amount = 1;
    int red = 255;
    int green = 0;
    int blue = 0;
    bool r_dir = false;
    bool g_dir = true;
    bool b_dir = false;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::KeyPressed)
            {
                int mouseX = sf::Mouse::getPosition().x;
                int mouseY = sf::Mouse::getPosition().y;

                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }

                else if (event.key.code == sf::Keyboard::Add)
                {
                    size += 1.f;
                    mass += 5.f;
                }
                else if (event.key.code == sf::Keyboard::Subtract) {
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
                        sf::Vector2f position(mouseX+15*i, mouseY);
                        sf::Color color = color_getter(red, green, blue, r_dir, g_dir, b_dir);
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