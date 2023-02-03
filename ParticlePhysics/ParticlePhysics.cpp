#include <iostream>
#include <SFML/Graphics.hpp>
#include "particle.h"
#include <cmath>
#include <sstream>
#include <algorithm>
#include <thread>
#include "UI.h"
#include "QTree.h"

/*

*****SOURCES*****
https://spicyyoghurt.com/tutorials/html5-javascript-game-development/collision-detection-physics
https://chat.openai.com/chat
https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html
https://en.wikipedia.org/wiki/Quadtree#:~:text=A%20quadtree%20is%20a%20tree,into%20four%20quadrants%20or%20regions.
*****SOURCES*****

*/

sf::Vector2f windowsize(1600, 900);
float dot(sf::Vector2f a, sf::Vector2f b) {
    return a.x * b.x + a.y * b.y;
}
sf::Color fire_color_updater(float temp) {
    
    float R, G, B;

    //float temp = temperature;
    if (temp <= 66) {
        // Cold: fade from black to red
        R = temp;
        R = 99.4708025861 * log(R) - 161.1195681661;
        G = 0;
        B = 0;
    }
    else if (temp <= 140) {
        // Warm: fade from red to yellow
        R = 255;
        G = (temp - 66) / (140 - 66) * 255;
        B = 0;
    }
    else if (temp <= 200) {
        // Hot: Fade from yellow to white
        R = 255;
        G = 255;
        B = (temp - 66) / (200 - 66) * 255;
    }
    else {
        // Hot: white
        R = 255;
        G = 255;
        B = 255;
    }

    if (R < 0) R = 0;
    if (R > 255) R = 255;
    if (G < 0) G = 0;
    if (G > 255) G = 255;
    if (B < 0) B = 0;
    if (B > 255) B = 255;

    sf::Color particle_color = sf::Color(R, G, B);
    return particle_color;
}
bool vertical_overlap(float y1, float y2, float r1, float r2) {
    if (abs(y1 - y2) - (r1 + r2) < 0) return true;
    return false;
}
bool horizontal_overlap(float x1, float x2, float r1, float r2) {
    if (abs(x1 - x2) - (r1 + r2) < 0) return true;
    return false;
}
void check_collisions1(std::vector<Particle*> particles, Particle* particle, sf::CircleShape* shape, int index, QuadTree* qt) {

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

    // Colliding with other particles neat it

    CircleBB boundary(particle->particle->getPosition(), particle->radius);
    std::vector<Point> points = qt->queryRange(boundary);

    //for (int i = 0; i < particles.size(); i++) {
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
                        
                        // Temperature transfers and then updates the colors
                        if (particle->type == "fire" && particles[i]->type == "fire") {
                            if (particle->temperature > particles[i]->temperature) {
                                particle->temperature--;
                                particles[i]->temperature++;
                                sf::Color first_color = fire_color_updater(particle->temperature);
                                sf::Color second_color = fire_color_updater(particles[i]->temperature);
                                particle->particle->setFillColor(first_color);
                                particles[i]->particle->setFillColor(second_color);
                            }
                            else if (particle->temperature < particles[i]->temperature) {
                                particle->temperature++;
                                particles[i]->temperature--;
                                sf::Color first_color = fire_color_updater(particle->temperature);
                                sf::Color second_color = fire_color_updater(particles[i]->temperature);
                                particle->particle->setFillColor(first_color);
                                particles[i]->particle->setFillColor(second_color);
                            }
                        }

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

                        v1->x -= impulse * m2 * vCollisionNorm.x;
                        v1->y -= impulse * m2 * vCollisionNorm.y;
                        v2->x += impulse * m1 * vCollisionNorm.x;
                        v2->y += impulse * m1 * vCollisionNorm.y;
                    }
                }
            }
        }
    }
}
void update_position(Particle* particle, sf::CircleShape* shape, int index, float deltaTime, float gravity) {
    
    // Move particle so far along the given path.
    float x = shape->getPosition().x;
    float y = shape->getPosition().y;

    particle->velocity->y += deltaTime * gravity * particle->mass;
    shape->setPosition(x + particle->velocity->x * deltaTime, y + particle->velocity->y * deltaTime);
}
void update_particle(std::vector<Particle*> particles, int index, float deltaTime, float gravity, QuadTree* qt) {
    update_position(particles[index], particles[index]->particle, index, deltaTime, gravity);
    check_collisions1(particles, particles[index], particles[index]->particle, index, qt);
}
sf::Color color_getter(int &r, int &g, int &b, bool &r_dir, bool &g_dir, bool &b_dir) {

    // How much the color gap is between particles
    int gap = 1;

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
float convert_y(float y) {
    return (windowsize.y * y / 1080);
}
sf::Vector2f convert_resolution(sf::Vector2f coordinates) {
    sf::Vector2f res(1920, 1080);
    float x = convert_x(coordinates.x);
    float y = convert_y(coordinates.y);
    return sf::Vector2f(x, y);
}
bool mouse_collide(sf::Vector2i mouse, sf::Vector2f position, sf::Vector2f size) {
    if (mouse.x > position.x && mouse.x < position.x + size.x && mouse.y > position.y && mouse.y < position.y + size.y) return true;
    return false;
}
void draw_qt(sf::RenderWindow* window, QuadTree* qt) {
    sf::RectangleShape boundary(sf::Vector2f(qt->boundary->w * 2 - 6, qt->boundary->h * 2 - 6));
    boundary.setOrigin(boundary.getGlobalBounds().width / 2, boundary.getGlobalBounds().height / 2);
    boundary.setPosition(windowsize.x / 2, windowsize.y / 2);
    boundary.setFillColor(sf::Color(0, 0, 0));
    boundary.setOutlineColor(sf::Color(255, 0, 0));
    boundary.setOutlineThickness(3);
    window->draw(boundary);
    if (qt->subdivided) {
        if (qt->northWest != NULL) draw_qt(window, qt->northWest);
        if (qt->northEast != NULL) draw_qt(window, qt->northEast);
        if (qt->southWest != NULL) draw_qt(window, qt->southWest);
        if (qt->southEast != NULL) draw_qt(window, qt->southEast);
    }
}
int main()
{
    int fps = 165;
    sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(windowsize.x, windowsize.y), "My Life is in Constant Torment :)");
    window->setFramerateLimit(fps);

    // Standard particle features
    int start_vel_x = 0;
    int start_vel_y = 0;
    int mass = 150;
    int radius = 5;
    int modifier = 1;
    int particle_amount = 1;
    int red = 255;
    int green = 0;
    int blue = 0;
    float gravity = 9.81f;
    int temperature = 15;
    bool rainbow_mode = false;

    std::string UI_render_type = "closed";
    UserInterface ui(windowsize);
    ui.create_UI(start_vel_x, start_vel_y, mass, radius, modifier, particle_amount, red, green, blue, gravity, temperature);
    std::vector<std::vector<sf::RectangleShape>> UI_vectors = ui.vectors;

    std::vector<Particle*> particles;
    std::vector<sf::Text*> texts = ui.texts;
    sf::CircleShape *particle_preview = ui.particle_preview;

    /*std::vector<sf::Vector3f> positions;
    for (int i = 0; i < particles.size(); i++) {
        positions.push_back(sf::Vector3f(particles[i]->particle->getPosition().x, particles[i]->particle->getPosition().y, particles[i]->radius));
    }*/

    float deltaTime = 1.f/fps;
    float substeps = 8.f;
    float subdt = deltaTime / substeps;
    bool r_dir = false;
    bool g_dir = true;
    bool b_dir = false;
    int thread_num = 4;

    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event)) 
        {
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Escape) {
                    window->close();
                }
                else if (event.key.code == sf::Keyboard::Delete) {
                    particles.clear();
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mouse = sf::Mouse::getPosition(*window);
                
                int eventtype = -1;
                if (UI_render_type == "closed") {

                    // Check if they are wanting to open the window
                    for (int i = 0; i < UI_vectors[0].size(); i++) {
                        sf::Vector2f size = UI_vectors[0][i].getSize();
                        sf::Vector2f pos = UI_vectors[0][i].getPosition();
                        if (mouse_collide(mouse, pos, size)) {
                            UI_render_type = "open";
                            for (int object = 0; object < UI_vectors.size(); object++) {
                                for (int rectangle = 0; rectangle < UI_vectors[object].size(); rectangle++) {
                                    sf::Vector2f pos = UI_vectors[object][rectangle].getPosition();
                                    UI_vectors[object][rectangle].setPosition(pos.x - convert_x(400), pos.y);
                                }
                            }
                            for (int object = 0; object < texts.size(); object++) {
                                sf::Vector2f pos = texts[object]->getPosition();
                                texts[object]->setPosition(pos.x - convert_x(400), pos.y);
                            }
                            sf::Vector2f pp_pos = particle_preview->getPosition();
                            particle_preview->setPosition(pp_pos.x - convert_x(400), pp_pos.y);
                            eventtype = i;
                            break;
                        }
                    }
                }
                else {
                    for (int i = 0; i < UI_vectors.size(); i++) {
                        // Check if they are wanting to close the window
                        for (int j = 0; j < UI_vectors[i].size(); j++) {
                            sf::Vector2f size = UI_vectors[i][j].getSize();
                            sf::Vector2f pos = UI_vectors[i][j].getPosition();
                            if (i == 0) {
                                if (mouse_collide(mouse, pos, size)) {
                                    UI_render_type = "closed";
                                    for (int object = 0; object < UI_vectors.size(); object++) {
                                        for (int rectangle = 0; rectangle < UI_vectors[object].size(); rectangle++) {
                                            sf::Vector2f pos = UI_vectors[object][rectangle].getPosition();
                                            UI_vectors[object][rectangle].setPosition(pos.x + convert_x(400), pos.y);
                                        }
                                    }
                                    for (int object = 0; object < texts.size(); object++) {
                                        sf::Vector2f pos = texts[object]->getPosition();
                                        texts[object]->setPosition(pos.x + convert_x(400), pos.y);
                                    }
                                    sf::Vector2f pp_pos = particle_preview->getPosition();
                                    particle_preview->setPosition(pp_pos.x + convert_x(400), pp_pos.y);
                                    eventtype = i;
                                    break;
                                }
                            }
                            else if (i == 1) {
                                if (mouse_collide(mouse, pos, size)) {
                                    eventtype = 1;
                                }
                            }
                            else if (i == 2) {
                                // Adjusting Red
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { red -= modifier; }
                                    else if (j == 2) { red += modifier; }
                                    if (red > 255) { red = 255; }
                                    if (red < 0) { red = 0; }
                                    particle_preview->setFillColor(sf::Color(red, green, blue));
                                    texts[0]->setString(std::to_string(red));
                                    eventtype = 2;
                                }
                            }
                            else if (i == 3) {
                                // Adjusting Green
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { green -= modifier; }
                                    else if (j == 2) { green += modifier; }
                                    if (green > 255) { green = 255; }
                                    if (green < 0) { green = 0; }
                                    particle_preview->setFillColor(sf::Color(red, green, blue));
                                    texts[1]->setString(std::to_string(green));
                                    eventtype = 3;
                                }
                            }
                            else if (i == 4) {
                                // Adjusting Blue
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { blue -= modifier; }
                                    else if (j == 2) { blue += modifier; }
                                    if (blue > 255) { blue = 255; }
                                    if (blue < 0) { blue = 0; }
                                    particle_preview->setFillColor(sf::Color(red, green, blue));
                                    texts[2]->setString(std::to_string(blue));
                                    eventtype = 4;
                                }
                            }
                            else if (i == 5) {
                                // Adjusting The Radius
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0 && radius > 1) { radius -= modifier; }
                                    else if (j == 2) { radius += modifier; }
                                    if (radius < 1) { radius = 1; }
                                    if (radius > windowsize.y / 2) { radius = windowsize.y / 2; }
                                    particle_preview->setRadius(radius);
                                    particle_preview->setPosition(convert_resolution(sf::Vector2f(1730 - radius, 70 - radius)));
                                    texts[3]->setString(std::to_string(radius));
                                    eventtype = 5;
                                }
                            }
                            else if (i == 6) {
                                // Adjusting The Mass
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { mass -= modifier; }
                                    else if (j == 2) { mass += modifier; }
                                    if (mass < 1) { mass = 1; }
                                    texts[4]->setString(std::to_string(mass));
                                    eventtype = 6;
                                }
                            }
                            else if (i == 7) {
                                // Adjusting The Velocity
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { start_vel_x -= modifier; }
                                    else if (j == 2) { start_vel_x += modifier; }
                                    else if (j == 3) { start_vel_y -= modifier; }
                                    else if (j == 5) { start_vel_y += modifier; }
                                    texts[5]->setString(std::to_string(start_vel_x));
                                    texts[6]->setString(std::to_string(start_vel_y));
                                    eventtype = 7;
                                }
                            }
                            else if (i == 8) {
                                // Adjusting The Number of Particles to Spawn
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { particle_amount -= modifier; }
                                    else if (j == 2) { particle_amount += modifier; }
                                    if (particle_amount < 1) { particle_amount = 1; }
                                    texts[7]->setString(std::to_string(particle_amount));
                                    eventtype = 8;
                                }
                            }
                            else if (i == 9) {
                                // Adjusting The Number of Particles to Spawn
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { modifier--; }
                                    else if (j == 2) { modifier++; }
                                    if (modifier < 1) { modifier = 1; }
                                    texts[8]->setString(std::to_string(modifier));
                                    eventtype = 9;
                                }
                            }
                            else if (i == 10) {
                                // Adjusting The Number of Particles to Spawn
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { gravity -= .01 * modifier; }
                                    else if (j == 2) { gravity += .01 * modifier; }
                                    if (gravity < 0) { gravity = 0; }
                                    texts[9]->setString(std::to_string(gravity));
                                    eventtype = 10;
                                }
                            }
                            else if (i == 11) {
                                // Adjusting The Number of Particles to Spawn
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) {
                                        if (rainbow_mode) {
                                            rainbow_mode = false;
                                            UI_vectors[11][j].setFillColor(sf::Color(215, 215, 185));
                                        }
                                        else {
                                            rainbow_mode = true;
                                            UI_vectors[11][j].setFillColor(sf::Color(215, 215, 0));
                                        }
                                    }
                                    eventtype = 11;
                                }
                            }
                            else if (i == 12) {
                                // Adjusting The Particle's Temperature
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { temperature -= modifier; }
                                    else if (j == 2) { temperature += modifier; }
                                    if (temperature < 0) temperature = 0;
                                    texts[10]->setString(std::to_string(temperature));
                                    eventtype = 12;
                                }
                            }
                        }
                    }
                }
                
                if (eventtype == -1) {

                    // Placing particle
                    for (int i = 0; i < particle_amount; i++) {
                        sf::Vector2f position(mouse.x + 15 * i, mouse.y);
                        
                        sf::Color color;
                        std::string type = "fire";
                        if (type != "fire") {
                            if (rainbow_mode) { color = color_getter(red, green, blue, r_dir, g_dir, b_dir); }
                            else { color = sf::Color(red, green, blue); }
                        }
                        else {
                            color = fire_color_updater(temperature);
                            red = color.r;
                            blue = color.b;
                            green = color.g;
                        }
                        sf::Vector2f* velocity = new sf::Vector2f(start_vel_x, start_vel_y);

                        // Updating the preview incase particles change colors willingly
                        particle_preview->setFillColor(sf::Color(red, green, blue));

                        Particle* particle = new Particle(radius, position, color, type, mass, velocity, temperature, 1);
                        particles.push_back(particle);
                    }
                    texts[0]->setString(std::to_string(red));
                    texts[1]->setString(std::to_string(green));
                    texts[2]->setString(std::to_string(blue));
                }
            }
        }

        window->clear();
        for (int min_substeps = substeps; min_substeps > 0; min_substeps--) {

            // Refactoring QuadTree
            RectangleBB* bounds = new RectangleBB(sf::Vector2f(windowsize.x / 2, windowsize.y / 2), int(windowsize.x/2), int(windowsize.y/2));
            QuadTree *qt = new QuadTree(bounds, 4);

            // Adds particles to QuadTree
            for (int i = 0; i < particles.size(); i++) {
                Point point(particles.at(i)->particle->getPosition(), particles.at(i));
                qt->insert(point);
            }

            // Displays QuadTree Boxes
            draw_qt(window, qt);

            // Draws Particles
            for (int i = 0; i < particles.size(); i++) {
                window->draw(*particles.at(i)->particle);
            }

            // Applies Updates to particles after drawing
            for (int i = 0; i < particles.size(); i++) {
                update_particle(particles, i, subdt, gravity, qt);
            }

            // Draws the particle UI
            for (int object = 0; object < UI_vectors.size(); object++) {
                for (int rectangle = 0; rectangle < UI_vectors[object].size(); rectangle++) {
                    window->draw(UI_vectors[object][rectangle]);
                    if (object == 1 && rectangle == UI_vectors[object].size() - 1) window->draw(*particle_preview);
                }
            }

            // Draws the text on top of UI
            for (int text = 0; text < texts.size(); text++) {
                window->draw(*texts.at(text));
            }
        }
        window->display();
    }
    return 0;
}