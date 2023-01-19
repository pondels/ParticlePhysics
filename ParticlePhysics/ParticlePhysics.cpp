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

sf::Vector2f windowsize(1280, 720);
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
    if (particlex <= radius) {
        particle->velocity->x *= -restitution;
        particle->particle->setPosition(radius, particley);
        particlex = radius;
    }
    else if (particlex >= windowsize.x - radius) {
        particle->velocity->x *= -restitution;
        particle->particle->setPosition(windowsize.x - radius, particley);
        particlex = windowsize.x - radius;
    }

    // Colliding with the ground
    if (particley >= windowsize.y - radius) {
        particle->velocity->y *= -restitution;
        particle->particle->setPosition(particlex, windowsize.y - radius);
    }

    // Colliding with the ceiling
    else if (particley <= radius) {
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
            bool ho = horizontal_overlap(x1, x2, r1, r2);
            if (ho) {
                bool vo = vertical_overlap(y1, y2, r1, r2);
                if (vo) {
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
                        sf::Vector2f relativeVelocity(v1->x - v2->x, v1->y - v2->y);

                        float speed = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;
                        if (speed < 0) {
                            break;
                        }
                        speed *= restitution;
                        float impulse = (2 * speed) / (m1 + m2);
                        float impulseScalar = dot(relativeVelocity, normal) * restitution / (1.0f / m1 + 1.0f / m2);
                        //sf::Vector2f impulse = impulseScalar * normal;
                        float dotProductTangent1 = v1->x * normal.x + v1->y * normal.y;
                        float dotProductTangent2 = v2->x * normal.x + v2->y * normal.y;

                        v1->x -= normal.x * impulse;
                        v1->y -= normal.y * impulse;
                        v2->x += normal.x * impulse;
                        v2->y += normal.y * impulse;
                    }
                }
            }
        }
    }
}
void update_position(Particle* particle, sf::CircleShape* shape, int index, float deltaTime) {
    
    // Move particle so far along the given path.
    float gravity = 9.81f;
    float x = shape->getPosition().x;
    float y = shape->getPosition().y;

    particle->velocity->y += deltaTime * gravity * particle->mass;
    shape->setPosition(x + particle->velocity->x * deltaTime, y + particle->velocity->y * deltaTime);
}
void update_particle(std::vector<Particle*> particles, int index, float deltaTime) {
    update_position(particles[index], particles[index]->particle, index, deltaTime);
    check_collisions1(particles, particles[index], particles[index]->particle, index);
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
float convert_x(float x) {
    return (windowsize.x * x / 1920);
}
float convert_y(float x) {
    return (windowsize.y * x / 1080);
}
sf::Vector2f convert_resolution(sf::Vector2f coordinates) {
    sf::Vector2f res(1920, 1080);
    float x = convert_x(coordinates.x);
    float y = convert_y(coordinates.y);
    return sf::Vector2f(x, y);
}
sf::RectangleShape make_bar(sf::Vector2f size, sf::Color color, sf::Vector2f position) {
    sf::RectangleShape rectangle(size);
    rectangle.setFillColor(color);
    rectangle.setPosition(position);
    return rectangle;
}
std::vector<std::vector<sf::RectangleShape>> create_UI(sf::RenderWindow& window, std::string render_type) {

    std::vector<std::vector<sf::RectangleShape>> vectors;

    vectors.push_back(std::vector<sf::RectangleShape>()); // Open/Close Button               [0]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Main background display for UI  [1]
    vectors.push_back(std::vector<sf::RectangleShape>()); // RGB boxes                       [2]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Box to Change Radius            [3]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Box to Change Mass              [4]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Boxes to Change Velocity        [5]

    // Declaring all the necessary colors
    sf::Color light_grey(75, 75, 75);
    sf::Color dark_grey(50, 50, 50);
    sf::Color pastel_red(196, 68, 61);
    sf::Color pastel_green(106, 204, 92);
    sf::Color pastel_blue(94, 89, 194);

    // Bars surrounding the Open/Close Button
    sf::Vector2f hb = convert_resolution(sf::Vector2f(50, 7));
    sf::Vector2f vb = convert_resolution(sf::Vector2f(7, 75));
    sf::Vector2f backdrop = convert_resolution(sf::Vector2f(50, 75));
        
    // Open/Close Button
    sf::RectangleShape rect1 = make_bar(hb, light_grey, convert_resolution(sf::Vector2f(1920 - 50, 75)));
    sf::RectangleShape rect2 = make_bar(vb, light_grey, convert_resolution(sf::Vector2f(1920 - 50, 75)));
    sf::RectangleShape rect3 = make_bar(hb, light_grey, convert_resolution(sf::Vector2f(1920 - 50, 75 + 75)));
    sf::RectangleShape rect4 = make_bar(backdrop, dark_grey, convert_resolution(sf::Vector2f(1920 - 50, 75)));

    // Bars surrounding the general UI
    sf::Vector2f bg = convert_resolution(sf::Vector2f(400, 625));
    sf::Vector2f hbm = convert_resolution(sf::Vector2f(400, 7));
    sf::Vector2f sb = convert_resolution(sf::Vector2f(7, 625));

    // General Box Environment
    sf::RectangleShape background = make_bar(bg, dark_grey, convert_resolution(sf::Vector2f(1920, 0)));
    sf::RectangleShape topbar =     make_bar(hbm, light_grey, convert_resolution(sf::Vector2f(1920, 0)));
    sf::RectangleShape bottombar =  make_bar(hbm, light_grey, convert_resolution(sf::Vector2f(1920, 625)));
    sf::RectangleShape sidebar =    make_bar(sb, light_grey, convert_resolution(sf::Vector2f(1920, 0)));

    // General Button Sizes
    sf::Vector2f plusminus =  convert_resolution(sf::Vector2f(20, 40));
    sf::Vector2f colorbox = convert_resolution(sf::Vector2f(40, 40));
    
    // RGB Buttons
    sf::RectangleShape red =    make_bar(colorbox,  pastel_red,   convert_resolution(sf::Vector2f(1920 + 75,  125)));
    sf::RectangleShape rplus =  make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 55,  125)));
    sf::RectangleShape rminus = make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 115, 125)));
    sf::RectangleShape green =  make_bar(colorbox,  pastel_green, convert_resolution(sf::Vector2f(1920 + 190, 125)));
    sf::RectangleShape gplus =  make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 230, 125)));
    sf::RectangleShape gminus = make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 170, 125)));
    sf::RectangleShape blue =   make_bar(colorbox,  pastel_blue,  convert_resolution(sf::Vector2f(1920 + 305, 125)));
    sf::RectangleShape bplus =  make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 345, 125)));
    sf::RectangleShape bminus = make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 285, 125)));

    // Radius, Size, and Velocity
    sf::RectangleShape radius = make_bar(colorbox, pastel_red, convert_resolution(sf::Vector2f(1920 + 75, 125)));
    sf::RectangleShape radplus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 55, 125)));
    sf::RectangleShape radminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 115, 125)));
    sf::RectangleShape mass = make_bar(colorbox, pastel_green, convert_resolution(sf::Vector2f(1920 + 190, 125)));
    sf::RectangleShape massplus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 230, 125)));
    sf::RectangleShape massminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 170, 125)));
    sf::RectangleShape velocityx = make_bar(colorbox, pastel_blue, convert_resolution(sf::Vector2f(1920 + 305, 125)));
    sf::RectangleShape velxplus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 345, 125)));
    sf::RectangleShape velxminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 285, 125)));
    sf::RectangleShape velocityy = make_bar(colorbox, pastel_blue, convert_resolution(sf::Vector2f(1920 + 305, 125)));
    sf::RectangleShape velyplus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 345, 125)));
    sf::RectangleShape velyminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 285, 125)));

    vectors[0].push_back(rect4);
    vectors[0].push_back(rect1);
    vectors[0].push_back(rect2);
    vectors[0].push_back(rect3);

    vectors[1].push_back(background);
    vectors[1].push_back(topbar);
    vectors[1].push_back(bottombar);
    vectors[1].push_back(sidebar);

    vectors[2].push_back(red);
    vectors[2].push_back(rminus);
    vectors[2].push_back(rplus);
    vectors[2].push_back(green);
    vectors[2].push_back(gminus);
    vectors[2].push_back(gplus);
    vectors[2].push_back(blue);
    vectors[2].push_back(bminus);
    vectors[2].push_back(bplus);
    return vectors;
}
int main()
{
    int fps = 165;
    sf::RenderWindow window(sf::VideoMode(windowsize.x, windowsize.y), "SFML works!");
    window.setFramerateLimit(fps);
 
    std::string UI_render_type = "closed";
    std::vector<std::vector<sf::RectangleShape>> UI_vectors = create_UI(window, UI_render_type);

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
                int mouseX = sf::Mouse::getPosition(window).x;
                int mouseY = sf::Mouse::getPosition(window).y;

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
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mouse = sf::Mouse::getPosition(window);
                
                bool place_particle = true;
                if (UI_render_type == "closed") {

                    // Check if they are wanting to open the window
                    for (int i = 0; i < UI_vectors[0].size(); i++) {
                        sf::Vector2f size = UI_vectors[0][i].getSize();
                        sf::Vector2f pos = UI_vectors[0][i].getPosition();
                        if (mouse.x > pos.x && mouse.y > pos.y && mouse.y < pos.y + size.y) {
                            UI_render_type = "open";
                            for (int object = 0; object < UI_vectors.size(); object++) {
                                for (int rectangle = 0; rectangle < UI_vectors[object].size(); rectangle++) {
                                    sf::Vector2f pos = UI_vectors[object][rectangle].getPosition();
                                    UI_vectors[object][rectangle].setPosition(pos.x - convert_x(400), pos.y);
                                }
                            }
                            place_particle = false;
                            break;
                        }
                    }
                }
                else {
                    // Changing Color
                    
                    // Increase Mass / Size
                    
                    // Changing Starting Velocity
                    
                    // Check if they are wanting to close the window
                    for (int i = 0; i < UI_vectors[0].size(); i++) {
                        sf::Vector2f size = UI_vectors[0][i].getSize();
                        sf::Vector2f pos = UI_vectors[0][i].getPosition();
                        if (mouse.x > pos.x && mouse.x < pos.x + size.x && mouse.y > pos.y && mouse.y < pos.y + size.y) {
                            UI_render_type = "closed";
                            for (int object = 0; object < UI_vectors.size(); object++) {
                                for (int rectangle = 0; rectangle < UI_vectors[object].size(); rectangle++) {
                                    sf::Vector2f pos = UI_vectors[object][rectangle].getPosition();
                                    UI_vectors[object][rectangle].setPosition(pos.x + convert_x(400), pos.y);
                                }
                            }
                            place_particle = false;
                            break;
                        }
                    }
                }
                
                if (place_particle) {
                    // Placing particle
                    for (int i = 0; i < particle_amount; i++) {
                        sf::Vector2f position(mouse.x + 15 * i, mouse.y);
                        sf::Color color = color_getter(red, green, blue, r_dir, g_dir, b_dir);
                        int grav_type = 1;
                        sf::Vector2f* velocity = new sf::Vector2f(0, 10);

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

        // Draws the particle UI
        for (int object = 0; object < UI_vectors.size(); object++) {
            for (int rectangle = 0; rectangle < UI_vectors[object].size(); rectangle++) {
                window.draw(UI_vectors[object][rectangle]);
            }
        }

        window.display();
    }

    return 0;
}