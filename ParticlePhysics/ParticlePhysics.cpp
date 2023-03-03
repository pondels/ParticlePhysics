#include <iostream>
#include <SFML/Graphics.hpp>
#include "particle.h"
#include <cmath>
#include <sstream>
#include <algorithm>
#include <thread>
#include "UI.h"
#include "QTree.h"
#include "shapes.h"

/*

*****SOURCES*****
https://spicyyoghurt.com/tutorials/html5-javascript-game-development/collision-detection-physics
https://chat.openai.com/chat
https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html
https://en.wikipedia.org/wiki/Quadtree#:~:text=A%20quadtree%20is%20a%20tree,into%20four%20quadrants%20or%20regions.
https://www.geeksforgeeks.org/2d-vector-in-cpp-with-user-defined-size/
http://arborjs.org/docs/barnes-hut
https://en.wikipedia.org/wiki/B%C3%A9zier_curve
https://gamedev.stackexchange.com/questions/37802/collision-detection-with-curves#:~:text=You%20can%20actually%20see%20if,closest%20point%20on%20the%20curve.
https://www.sfml-dev.org/tutorials/2.5/graphics-view.php
*****SOURCES*****

*/

// Tracks the biggest radius on screen
int biggest_radius = 0;
const float grav_const = 6.6743 * pow(10, -6);
const float pi = 3.14159265;
float gravity = 0.0f;
sf::Vector2f windowsize(1280, 720);
sf::Vector2f quadtreesize(1280, 720);

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
template <typename T> void line_collision(Particle* particle, T lines) {

    float restitution = .9f;
    sf::Vector2f point = particle->particle->getPosition();
    float radius = particle->radius;

    for (auto& line : lines) {
        bool isCollision = false;
        sf::VertexArray* shape = line.shape;
        std::size_t numVertices = shape->getVertexCount();
        bool colliding = false;
        float closest_dist = -1;
        sf::Vector2f closest_pos;

        // Check if the point is inside the vertex array
        for (int i = 0; i < numVertices; i++)
        {

            // Particles Colliding
            sf::Vector2f pos = (*shape)[i].position;
            float distance = (pos.x - point.x) * (pos.x - point.x) + (pos.y - point.y) * (pos.y - point.y);

            // Colliding
            if (distance < radius * radius) {
                colliding = true;
                if (distance < closest_dist || closest_dist == -1) {
                    closest_dist = distance;
                    closest_pos = pos;
                }

                // Updates particles velocity
                sf::Vector2f lineVec = (*shape)[(i + 1) % numVertices].position - pos;
                sf::Vector2f normal = sf::Vector2f(-lineVec.y, lineVec.x);
                float length = sqrt(normal.x * normal.x + normal.y * normal.y);
                normal /= length;
                sf::Vector2f velocity = *particle->velocity;
                float dot_product = velocity.x * normal.x + velocity.y * normal.y;
                sf::Vector2f new_velocity = -2.f * dot_product * normal + velocity;
                particle->velocity->x = new_velocity.x;
                particle->velocity->y = new_velocity.y;
            }
        }

        if (colliding) {
            particle->velocity->y *= -restitution;

            float set_x;
            float set_y;
            set_x = point.x;
            if (closest_pos.y > point.y) set_y = closest_pos.y - radius;
            else set_y = closest_pos.y + radius;
            particle->particle->setPosition(set_x, set_y);
        }
    }
}
void check_collisions(std::vector<Particle*>* particles, Particle* particle, sf::CircleShape* shape, int index, QuadTree* qt, std::vector<Line> lines, std::vector<Bezier_Curve> curves) {
    // Update Direction & Speed of particle based on collisions.

    // Rate at which energy is lost against the wall/floor
    float restitution = .9 - (particle->viscosity/500);
    float friction = 0.9 + 0.099 * (1 - particle->viscosity / 500);

    float radius = shape->getGlobalBounds().height / 2;
    float particlex = shape->getPosition().x;
    float particley = shape->getPosition().y;

    if (gravity != 0) {
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
            particle->velocity->x *= friction;
            particle->particle->setPosition(particlex, windowsize.y - radius);
        }

        // Colliding with the ceiling
        else if (particley < radius) {
            particle->velocity->y *= -restitution;
            particle->particle->setPosition(particlex, radius);
        }
    }

    // Particle colliding with Line/Curve objects
    line_collision(particle, lines);
    line_collision(particle, curves);

    // Colliding with other particles neat it
    RectangleBB boundary(particle->particle->getPosition(), radius + biggest_radius, radius + biggest_radius);
    std::vector<Point> points;
    qt->queryRange(boundary, &points);

    float x2 = shape->getPosition().x;
    float y2 = shape->getPosition().y;
    float r2 = particle->radius;
    double m2 = particle->mass;
    sf::Vector2f* v2 = particle->velocity;

    // Particles to remove if consumed, destroyed, eaten, chomped, devoured, incinerated, blown up, thrown into the eternal abyss, or banished to the shadow realm
    std::vector<int> remove_indices;


    for (int i = 0; i < points.size(); i++) {
        int p_i = points.at(i).index;

        if (p_i != index) {

            float x1 = (*particles)[p_i]->particle->getPosition().x;
            float y1 = (*particles)[p_i]->particle->getPosition().y;
            float r1 = (*particles)[p_i]->radius;
            double m1 = (*particles)[p_i]->mass;
            sf::Vector2f* v1 = (*particles)[p_i]->velocity;

            if (horizontal_overlap(x1, x2, r1, r2)) {
                if (vertical_overlap(y1, y2, r1, r2)) {
                    float squaredistance = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
                    if (squaredistance < ((r1 + r2) * (r1 + r2)) && squaredistance != 0) {

                        // Particle should be consumed
                        if (particle->consume || (*particles)[p_i]->consume) {

                            // Adding mass and volume to the circle that nom-nom's the particle
                            float area1 = pi * pow(particle->radius, 2);
                            float area2 = pi * pow((*particles)[p_i]->radius, 2);
                            float new_mass = particle->mass + (*particles)[p_i]->mass;
                            float new_radius = sqrt((area1 + area2) / pi);

                            // Add radius and mass to the particle the consumes and remove the other particle
                            if (particle->consume) {
                                // Banishing the particle to the shadow realm
                                remove_indices.push_back(p_i);
                                particle->mass = new_mass;
                                particle->radius = new_radius;
                                particle->particle->setRadius(new_radius);
                                particle->particle->setOrigin(new_radius, new_radius);
                            }
                            else {
                                // The particle has been consumed so there's no reason for it to meet new friends
                                remove_indices.push_back(index);
                                (*particles)[p_i]->mass = new_mass;
                                (*particles)[p_i]->radius = new_radius;
                                (*particles)[p_i]->particle->setRadius(new_radius);
                                particle->particle->setOrigin(new_radius, new_radius);
                                break;
                            }
                        }
                        else {
                            // Temperature transfers and then updates the colors
                            if (particle->type == "fire" && (*particles)[p_i]->type == "fire") {
                                if (particle->temperature > (*particles)[p_i]->temperature) {
                                    particle->temperature--;
                                    (*particles)[p_i]->temperature++;
                                    sf::Color first_color = fire_color_updater(particle->temperature);
                                    sf::Color second_color = fire_color_updater((*particles)[p_i]->temperature);
                                    particle->particle->setFillColor(first_color);
                                    (*particles)[p_i]->particle->setFillColor(second_color);
                                }
                                else if (particle->temperature < (*particles)[p_i]->temperature) {
                                    particle->temperature++;
                                    (*particles)[p_i]->temperature--;
                                    sf::Color first_color = fire_color_updater(particle->temperature);
                                    sf::Color second_color = fire_color_updater((*particles)[p_i]->temperature);
                                    particle->particle->setFillColor(first_color);
                                    (*particles)[p_i]->particle->setFillColor(second_color);
                                }
                            }

                            float distance = sqrtf(squaredistance);
                            if (distance > 0.0001) {
                                float overlap = (distance - r1 - r2) / 2.f;

                                // If inside a particle, break out
                                float moveX = (overlap * (x1 - x2) / distance);
                                float moveY = (overlap * (y1 - y2) / distance);

                                (*particles)[p_i]->particle->setPosition(x1 - moveX, y1 - moveY);
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
    }


    if (remove_indices.size() > 0) {
        // Removes all the unwanted particles
        sort(remove_indices.begin(), remove_indices.end(), std::greater<int>());
        for (int index : remove_indices) if (index < particles->size()) particles->erase(particles->begin() + index);
    }
}
void update_position(Particle* particle, sf::CircleShape* shape, int index, float deltaTime, float gravity, QuadTree* qt) {

    // Move particle so far along the given path.
    float x = shape->getPosition().x;
    float y = shape->getPosition().y;

    particle->velocity->y += deltaTime * gravity * particle->mass;
    shape->setPosition(x + particle->velocity->x * deltaTime, y + particle->velocity->y * deltaTime);
}
void space_update_position(std::vector<Particle*> particles, float deltaTime, Barnes_Hut* space_qt) {

    std::vector<std::tuple<float, float>> transformations;

    const float softener = .1f;
    const float GRAV_CONST = 25.f;

    for (int i = 0; i < particles.size(); i++) {
        float x_shift = 0;
        float y_shift = 0;
        space_qt->calculate_force(i, particles[i], x_shift, y_shift);
        std::tuple<float, float> transformation(x_shift, y_shift);
        transformations.push_back(transformation);
    }

    // Updates the particles with their respective x and y shift
    for (int i = 0; i < particles.size(); i++) {
        sf::Vector2f pos = particles[i]->particle->getPosition();
        float mass = particles[i]->mass;
        particles[i]->velocity->x += deltaTime * std::get<0>(transformations[i]) / mass;
        particles[i]->velocity->y += deltaTime * std::get<1>(transformations[i]) / mass;
        particles[i]->particle->setPosition(pos.x + particles[i]->velocity->x * deltaTime, pos.y + particles[i]->velocity->y * deltaTime);
    }
}
void wind_sim(std::vector<Particle*>* particles, float horiztonal_blow, float vertical_blow) {
    for (auto& particle : (*particles)) {
        particle->velocity->x += horiztonal_blow / 5;
        particle->velocity->y += vertical_blow / 5;
    }
}
void update_particles(std::vector<Particle*>* particles, float deltaTime, float gravity, QuadTree* collisions_qt, Barnes_Hut* space_qt, std::vector<Line> lines, std::vector<Bezier_Curve> curves) {
    
    if (gravity != 0) {
        for (int i = 0; i < particles->size(); i++) {
            update_position((*particles)[i], (*particles)[i]->particle, i, deltaTime, gravity, collisions_qt);
            check_collisions(particles, (*particles)[i], (*particles)[i]->particle, i, collisions_qt, lines, curves);
        }
    }
    else {
        space_update_position((*particles), deltaTime, space_qt);
        for (int i = 0; i < particles->size(); i++) {
            check_collisions(particles, (*particles)[i], (*particles)[i]->particle, i, collisions_qt, lines, curves);
        }
    }
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
int main()
{
    int fps = 165;
    sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(windowsize.x, windowsize.y), "My Life is in Constant Torment :)");
    window->setFramerateLimit(fps);
    sf::View view(window->getDefaultView());
    window->setView(view);

    // Offset of the view to properly interact with the window
    float view_horizonal = 0.0f;
    float view_vertical  = 0.0f;
    float view_zoom = 1.f;

    // Standard particle features
    int start_vel_x = 0;
    int start_vel_y = 0;
    int mass = 150;
    int radius = 15;
    int modifier = 1;
    int particle_amount = 1;
    int red = 255;
    int green = 0;
    int blue = 0;
    int temperature = 15;
    bool rainbow_mode = true;
    float viscosity = 70;
    bool consume = false;
    bool explode = false;

    // Wind Garbage
    bool wind_enabled = false;
    float horizontal_blow = 0.0f;
    float vertical_blow = 0.0f;

    // What mode the user is on
    bool draw_line = false;
    bool draw_curve = false;
    bool draw_particles = true;

    // Sets the vectors up for drawing custom shapes
    std::vector<Line> lines;
    std::vector<Bezier_Curve> curves;

    std::string UI_render_type = "closed";
    UserInterface ui(windowsize);
    ui.create_UI(start_vel_x, start_vel_y, mass, radius, modifier, particle_amount, red, green, blue, gravity, temperature);
    std::vector<std::vector<sf::RectangleShape>> UI_vectors = ui.vectors;

    std::vector<Particle*>* particles = new std::vector<Particle*>;
    std::vector<sf::Text*> texts = ui.texts;
    sf::CircleShape *particle_preview = ui.particle_preview;

    float deltaTime = 1.f/fps;
    float substeps = 10.f;
    float subdt = deltaTime / substeps;
    bool r_dir = false;
    bool g_dir = true;
    bool b_dir = false;

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

                // Pan Around Screen
                else if (event.key.code == sf::Keyboard::Left) {
                    view.move(-10.0f, 0.0f);
                    view_horizonal -= 10;
                }
                else if (event.key.code == sf::Keyboard::Right) {
                    view.move(10.0f, 0.0f);
                    view_horizonal += 10;
                }
                else if (event.key.code == sf::Keyboard::Up) {
                    view.move(0.0f, -10.0f);
                    view_vertical -= 10;
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    view.move(0.0f, 10.0f);
                    view_vertical += 10;
                }
                
                else if (event.key.code == sf::Keyboard::Delete) {
                    for (int i = 0; i < particles->size(); i++) {
                        delete (*particles)[i];
                    }
                    particles->clear();
                    lines.clear();
                    curves.clear();
                    biggest_radius = 0;
                }
                else if (event.key.code == sf::Keyboard::L) {
                    draw_line = true;
                    draw_curve = false;
                    draw_particles = false;
                }
                else if (event.key.code == sf::Keyboard::C) {
                    draw_line = false;
                    draw_curve = true;
                    draw_particles = false;
                }
                else if (event.key.code == sf::Keyboard::P) {
                    draw_line = false;
                    draw_curve = false;
                    draw_particles = true;
                }
                else if (event.key.code == sf::Keyboard::W) {
                    wind_enabled = wind_enabled ? false : true;
                }
                
                // Custom Particles 0 - 9 on keyboard
                else if (event.key.code == 26) { // Consumed other particles
                    consume = consume ? false : true;
                }
                else if (event.key.code == 27) { // Explodes into smaller particles
                    // Particles explode into an undetermined amount of particles on contact if fast enough
                    explode = explode ? false : true;
                }
                else if (event.key.code == 28) { // Gives a particle negative mass
                    // Negative Mass
                    mass = -mass;
                }
                else if (event.key.code == 29) {
                    // Electricity?
                }
                else if (event.key.code == 30) {
                    // Radioactive
                    
                }
                else if (event.key.code == 31) {
                    // Magenetism
                }
                else if (event.key.code == 32) {
                    // Teleportation
                }
                else if (event.key.code == 33) {
                    // Particle Swap
                }
                else if (event.key.code == 34) {

                }
                else if (event.key.code == 35) {

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
                            else if (i == 13) {
                                // Adjusting the Wind values
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { horizontal_blow--; }
                                    if (j == 2) { horizontal_blow++; }
                                    if (j == 3) { vertical_blow--; }
                                    if (j == 5) { vertical_blow++; }
                                }
                            }
                        }
                    }
                }
                
                if (eventtype == -1) {

                    if (draw_line) {
                        
                        // Checks if the last line was complete
                        if (lines.size() == 0) {
                            sf::VertexArray* new_vertex = new sf::VertexArray(sf::LineStrip, 50);
                            Line new_line(new_vertex, 50);
                            lines.push_back(new_line);
                        }
                        else if (lines.back().drawable) {
                            sf::VertexArray* new_vertex = new sf::VertexArray(sf::LineStrip, 50);
                            Line new_line(new_vertex, 50);
                            lines.push_back(new_line);
                        }
                        lines.back().add_point(sf::Vector2i(mouse.x, mouse.y));
                    }
                    else if (draw_curve) {

                        // Checks if the last line was complete
                        if (curves.size() == 0) {
                            sf::VertexArray* new_vertex = new sf::VertexArray(sf::LineStrip, 50);
                            Bezier_Curve new_curve(new_vertex, 50);
                            curves.push_back(new_curve);
                        }
                        else if (curves.back().drawable) {
                            sf::VertexArray* new_vertex = new sf::VertexArray(sf::LineStrip, 50);
                            Bezier_Curve new_curve(new_vertex, 50);
                            curves.push_back(new_curve);
                        }
                        curves.back().add_point(sf::Vector2i(mouse.x, mouse.y));
                    }
                    else if (draw_particles) {
                        // Updates the biggest particle on screen
                        if (radius > biggest_radius) biggest_radius = radius;

                        // Placing particle
                        for (int i = 0; i < particle_amount; i++) {

                            sf::Vector2f position(
                                (mouse.x + i + view_horizonal) * view_zoom,
                                (mouse.y + i + view_vertical)  * view_zoom
                            );

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
                            sf::Vector2f velocity(start_vel_x, start_vel_y);

                            // Updating the preview incase particles change colors willingly
                            particle_preview->setFillColor(sf::Color(red, green, blue));

                            Particle* particle = new Particle(radius, position, color, type, mass, velocity, temperature, viscosity, consume, explode);
                            particles->push_back(particle);
                        }
                        texts[0]->setString(std::to_string(red));
                        texts[1]->setString(std::to_string(green));
                        texts[2]->setString(std::to_string(blue));
                    }
                }
            }
            // Zooming in and out
            if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.delta > 0) {
                    view.zoom(0.8f);
                    view_zoom *= .8f;
                }
                else if (event.mouseWheelScroll.delta < 0) {
                    view.zoom(1.25f);
                    view_zoom *= 1.25f;
                }
            }
        }

        window->setView(view);
        window->clear();

        for (int min_substeps = substeps; min_substeps > 0; min_substeps--) {

            // Boundary to the quadtrees
            //RectangleBB bounds(sf::Vector2f(0, 0));
            RectangleBB bounds(sf::Vector2f(windowsize.x / 2, windowsize.y / 2), int(windowsize.x / 2), int(windowsize.y / 2));
            if (gravity == 0) {
                float max_x = 0.0f;
                float max_y = 0.0f;
                for (auto& particle : (*particles)) {
                    sf::Vector2f pos = particle->particle->getPosition();
                    if (pos.x + particle->radius > max_x) max_x = pos.x + particle->radius;
                    if (pos.y + particle->radius > max_y) max_y = pos.y + particle->radius;
                }
                quadtreesize.x = max_x;
                quadtreesize.y = max_y;
                bounds.x = quadtreesize.x / 2;
                bounds.y = quadtreesize.y / 2;
                bounds.w = quadtreesize.x / 2;
                bounds.h = quadtreesize.y / 2;
            }
            // Refactoring QuadTrees
            QuadTree *collisions_qt = new QuadTree(bounds, 4);
            Barnes_Hut *space_qt = new Barnes_Hut(bounds, 1);

            // Updates particles positions in case they are on the same pixel
            //  This prevents a stack overflow with the space partitioner
            sf::Vector2f curr_xy(-1, -1);
            for (auto& particle : (*particles)) {
                if (curr_xy.x == -1) {
                    curr_xy = particle->particle->getPosition();
                }
                else {

                    sf::Vector2f pos = particle->particle->getPosition();

                    if (curr_xy.x == pos.x && curr_xy.y == pos.y) {
                        pos.x++;
                        pos.y++;
                        particle->particle->setPosition(pos.x, pos.y);
                    }
                    curr_xy = pos;
                }
            }

            // Adds particles to QuadTree
            for (int i = 0; i < particles->size(); i++) {
                sf::Vector2f pos = (*particles)[i]->particle->getPosition();
                Point point(pos, i, (*particles)[i]->mass);
                collisions_qt->insert(point);
                if (gravity == 0) space_qt->insert(point);
            }

            // Draws Particles
            for (auto& particle : (*particles)) window->draw(*particle->particle);

            // Draws all custom lines and curves
            for (auto& line : lines) {
                if (line.drawable) {
                    window->draw(*line.shape);
                }
            }
            for (auto& curve : curves) {
                if (curve.drawable) {
                    window->draw(*curve.shape);
                }
            }

            update_particles(particles, subdt, gravity, collisions_qt, space_qt, lines, curves);

            // Wind Physics
            if (wind_enabled) wind_sim(particles, horizontal_blow, vertical_blow);

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

            delete collisions_qt;
            delete space_qt;
        }
        window->display();
    }
    return 0;
}