#include <iostream>
#include <SFML/Graphics.hpp>
#include "particle.h"
#include <cmath>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <random>
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
https://cplusplus.com/reference/random/?kw=%3Crandom%3E
*****SOURCES*****

*/

// Tracks the biggest radius on screen
int biggest_radius = 0;
const float grav_const = 6.6743 * pow(10, -6);
const float pi = 3.14159265;
float gravity = 0.0f;
const float softener = .1f;
const float GRAV_CONST = 25.f;
sf::Vector2f windowsize(1280, 720);
sf::Vector2f quadtreesize(1280, 720);

float random_number_generator(std::tuple<int, int> range = std::tuple<int, int>(1, 100)) {

    // I googled a few methods. Chat GPT3 came in clutch with a better solution.
    // I tried looking for where it may have found this solution but genuinely found nothing.
    // Closest I found was from the cplusplus site, so I linked that

    // Grabs the time and converts the time to microsecond transfer
    auto time = std::chrono::system_clock::now();
    auto msec = std::chrono::time_point_cast<std::chrono::microseconds>(time).time_since_epoch().count();

    // Randomize a random number 0 - 100 using the difference in microseconds
    std::mt19937_64 rng(msec);
    std::uniform_int_distribution<int> dist(std::get<0>(range), std::get<1>(range));

    // This is where the magic happens
    double random_number = dist(rng);

    return random_number;
}
float dots(sf::Vector2f a, sf::Vector2f b) {
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
void chernobyl_particle(Particle* particle, int number) {

    //      Makes a particle smaller upon impact
    //      Makes a particle less dense upon impact
    //      Makes a particle radioactive upon impact - spreads like the plague
    //      Can activate and deactivate certain abilities in a particle

    switch (number) {

    // Shrinks Particle
    case 0: {
        float radius = particle->radius;
        if (radius > 2) {
            radius--;
            particle->radius = radius;
            particle->particle->setRadius(radius);
            particle->particle->setOrigin(radius, radius);
        }
    }

    // Makes particle less dense
    case 100: particle->mass--;

    // Makes particle radioactive
    case 200: particle->radioactive = true;
    
    // Everything below messes with abilities and properties
    case 300:  particle->type = (particle->type == "fire") ? "normal" : "fire";
    case 400:  particle->temperature += 10;
    case 500:  particle->swap = (particle->swap) ? false : true;
    case 600:  particle->iridescent = (particle->iridescent) ? false : true;
    case 700:  particle->consume = (particle->consume) ? false : true;
    }

}
void inherit_properties(Particle* particle1, Particle* particle2) {

    // First particle -  Particle receiving properties
    // Second particle - Particle transferring properties

    particle1->consume = (particle2->consume) ? true : particle1->consume;
    //particle1->type = (particle2->type == "fire") ? "fire" : particle1->type;
    //particle1->viscosity *= particle2->viscosity;
    particle1->temperature += particle2->temperature;
    particle1->explode = (particle2->explode) ? true : particle1->explode;
    particle1->teleportation = (particle2->teleportation) ? true : particle1->teleportation;
    particle1->swap = (particle2->swap) ? true : particle1->swap;
    particle1->iridescent = (particle2->iridescent) ? true : particle1->iridescent;
    sf::Color color1 = particle1->particle->getFillColor();
    sf::Color color2 = particle2->particle->getFillColor();
    color1.r += color2.r - color1.r;
    color1.g += color2.g - color1.g;
    color1.b += color2.b - color1.b;
    particle1->particle->setFillColor(color1);
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
    float friction = 0.9 + 0.099 * (1.f - particle->viscosity / 500.f);

    float radius = shape->getGlobalBounds().height / 2;
    float particlex = shape->getPosition().x;
    float particley = shape->getPosition().y;

    if (particlex < radius) {
        particle->velocity->x *= -restitution;
        particle->particle->setPosition(radius, particley);
        particlex = radius;
    }

    // Colliding with the ceiling
    if (particley < radius) {
        particle->velocity->y *= -restitution;
        particle->particle->setPosition(particlex, radius);
    }

    if (gravity != 0) {
        // Colliding with the other wall
        if (particlex > windowsize.x - radius) {
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
    double m2 = abs(particle->mass);
    sf::Vector2f* v2 = particle->velocity;

    // Particles to remove if consumed, destroyed, eaten, chomped, devoured, incinerated, blown up, thrown into the eternal abyss, or banished to the shadow realm
    std::vector<int> remove_indices;


    for (int i = 0; i < points.size(); i++) {
        int p_i = points.at(i).index;

        if (p_i != index) {

            float x1 = (*particles)[p_i]->particle->getPosition().x;
            float y1 = (*particles)[p_i]->particle->getPosition().y;
            float r1 = (*particles)[p_i]->radius;
            double m1 = abs((*particles)[p_i]->mass);
            sf::Vector2f* v1 = (*particles)[p_i]->velocity;

            if (horizontal_overlap(x1, x2, r1, r2)) {
                if (vertical_overlap(y1, y2, r1, r2)) {
                    float squaredistance = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
                    if (squaredistance < ((r1 + r2) * (r1 + r2)) && squaredistance != 0) {

                        // Handles Radioactivity
                        if ((*particles)[p_i]->radioactive) {
                            float random_number = random_number_generator(std::tuple<int, int>(0, 700));
                            chernobyl_particle(particle, random_number);
                        }
                        if (particle->radioactive) {
                            float random_number = random_number_generator(std::tuple<int, int>(0, 700));
                            chernobyl_particle((*particles)[p_i], random_number);
                        }

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
                                inherit_properties(particle, (*particles)[p_i]);
                            }
                            else {
                                // The particle has been consumed so there's no reason for it to meet new friends
                                remove_indices.push_back(index);
                                (*particles)[p_i]->mass = new_mass;
                                (*particles)[p_i]->radius = new_radius;
                                (*particles)[p_i]->particle->setRadius(new_radius);
                                particle->particle->setOrigin(new_radius, new_radius);
                                inherit_properties((*particles)[p_i], particle);
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

                                // Handles Explostions
                                if ((*particles)[p_i]->explode) {

                                    float private_speed = sqrt(v1->x * v1->x + v1->y * v1->y);
                                    float radius = (*particles)[p_i]->radius;
                                    float area = pi * pow(radius, 2);

                                    int limit = 1;
                                    while (true) {
                                        float test_area = area / limit;
                                        float test_radius = test_area / (pi * radius);
                                        if (test_radius <= 1) break;
                                        limit++;
                                    }

                                    int particle_amt = private_speed / 500;

                                    // Chooses the smallest amount so we don't over-divide
                                    particle_amt = (particle_amt > limit) ? limit: particle_amt;

                                    // Splits the particle if it can split
                                    if (particle_amt > 1) {
                                        
                                        area = area / particle_amt;
                                        radius = sqrt(area / pi);
                                        float mass = (*particles)[p_i]->mass / particle_amt;

                                        remove_indices.push_back(p_i);
                                        for (int n = 0; n < particle_amt; n++) {

                                            Particle* new_particle = new Particle(radius, (*particles)[p_i]->particle->getPosition(), (*particles)[p_i]->particle->getFillColor(), (*particles)[p_i]->type, mass, sf::Vector2f((*particles)[p_i]->velocity->x, (*particles)[p_i]->velocity->y), (*particles)[p_i]->temperature, (*particles)[p_i]->viscosity, (*particles)[p_i]->consume, (*particles)[p_i]->explode, (*particles)[p_i]->teleportation, (*particles)[p_i]->swap, (*particles)[p_i]->iridescent, (*particles)[p_i]->radioactive);
                                            particles->push_back(new_particle);
                                        }
                                    }
                                }
                                if (particle->explode) {

                                    float private_speed = sqrt(v2->x * v2->x + v2->y * v2->y);
                                    float radius = particle->radius;
                                    float area = pi * pow(radius, 2);

                                    int limit = 1;
                                    while (true) {
                                        float test_area = area / limit;
                                        float test_radius = test_area / (pi * radius);
                                        if (test_radius <= 1) break;
                                        limit++;
                                    }

                                    int particle_amt = private_speed / 500;

                                    // Chooses the smallest amount so we don't over-divide
                                    particle_amt = (particle_amt > limit) ? limit : particle_amt;

                                    if (particle_amt > 1) {

                                        area = area / particle_amt;
                                        radius = sqrt(area / pi);
                                        float mass = (*particles)[p_i]->mass / particle_amt;

                                        remove_indices.push_back(index);
                                        for (int n = 0; n < particle_amt; n++) {
                                            Particle* new_particle = new Particle(radius, particle->particle->getPosition(), particle->particle->getFillColor(), particle->type, mass, sf::Vector2f(particle->velocity->x, particle->velocity->y), particle->temperature, particle->viscosity, particle->consume, particle->explode, particle->teleportation, particle->swap, particle->iridescent, particle->radioactive);
                                            particles->push_back(new_particle);
                                        }

                                        // If the main particle explodes, we don't want to use the original anymore
                                        break;
                                    }
                                }
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
void space_update_position(std::vector<Particle*> particles, float deltaTime, Barnes_Hut* space_qt, int i) {

    float x_shift = 0;
    float y_shift = 0;
    space_qt->calculate_force(i, particles[i], x_shift, y_shift);
    std::tuple<float, float> transformation(x_shift, y_shift);

    sf::Vector2f pos = particles[i]->particle->getPosition();
    float mass = particles[i]->mass;
    particles[i]->velocity->x += deltaTime * std::get<0>(transformation) / mass;
    particles[i]->velocity->y += deltaTime * std::get<1>(transformation) / mass;
    particles[i]->particle->setPosition(pos.x + particles[i]->velocity->x * deltaTime, pos.y + particles[i]->velocity->y * deltaTime);
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
        for (int i = 0; i < particles->size(); i++) {
            space_update_position((*particles), deltaTime, space_qt, i);
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
int main()
{
    int fps = 60;
    float zoom_val = 1;
    int num_threads = 4;
    float substeps = 8.f;
    float deltaTime = 1.f / fps;
    float subdt = deltaTime / substeps;

    sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(windowsize.x, windowsize.y), "My Life is in a Constant Pendulum of Torment :)");
    window->setFramerateLimit(fps);
    sf::View view(window->getDefaultView());
    window->setView(view);

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
    int temperature = 15;
    bool rainbow_mode = false;
    float viscosity = 70;
    bool consume = false;
    bool explode = false;
    bool teleportation = false;
    bool particle_swap = false;
    bool iridescent = false;
    bool radioactive = false;
    std::string type = "normal";

    // Used for dividing easier
    double percent_divisor = (1/100.f) * (1/float(fps)) * (1/substeps);

    // Chances of certain abilities happening (1% is the lowest you can go for now)
    double teleport_chance =   1.f * percent_divisor; // 1% chance per second
    double swap_chance =       1.f * percent_divisor; // 1% chance per second
    double iridescent_chance = 1.f * percent_divisor; // 1% chance per second

    // Wind Garbage
    bool wind_enabled = false;
    float horizontal_blow = 0.0f;
    float vertical_blow = 0.0f;

    // What mode the user is on
    bool draw_line = false;
    bool draw_curve = false;
    bool draw_particles = true;
    bool move_window = false;

    // Sets the vectors up for drawing custom shapes
    std::vector<Line> lines;
    std::vector<Bezier_Curve> curves;

    std::string UI_render_type = "closed";
    UserInterface ui(windowsize);
    ui.create_UI(start_vel_x, start_vel_y, mass, radius, modifier, particle_amount, red, green, blue, gravity, temperature);
    std::vector<std::vector<sf::RectangleShape*>> UI_vectors = ui.vectors;

    std::vector<Particle*>* particles = new std::vector<Particle*>;
    std::vector<sf::Text*> texts = ui.texts;
    std::vector<sf::CircleShape*> preview_particles = ui.preview_particles;

    bool r_dir = false;
    bool g_dir = true;
    bool b_dir = false;

    // Preview Values
    bool grow = true;
    bool grow_rad = true;
    int preview_temp = 0;
    bool heating = true;

    sf::Vector2f prev_teleport_min(5, 28);
    sf::Vector2f prev_tele_fifth_max(280, 310);
    sf::Vector2f prev_tele_sixth_max(315, 345);

    sf::VertexArray* plv = new sf::VertexArray(sf::LineStrip, 50);
    Line preview_line(plv, 50);
    preview_line.add_point(sf::Vector2i(convert_resolution(sf::Vector2f(7, 130))));
    preview_line.add_point(sf::Vector2i(convert_resolution(sf::Vector2f(45, 163))));
    sf::VertexArray* pcv = new sf::VertexArray(sf::LineStrip, 50);
    Bezier_Curve preview_curve(pcv, 50);
    preview_curve.add_point(sf::Vector2i(convert_resolution(sf::Vector2f(7, 183))));
    preview_curve.add_point(sf::Vector2i(convert_resolution(sf::Vector2f(7, 213))));
    preview_curve.add_point(sf::Vector2i(convert_resolution(sf::Vector2f(45, 213))));

    // Wind
    sf::Texture wind_texture;
    sf::Texture clear_texture;
    if (!wind_texture.loadFromFile("C:\\Users\\yeckr\\OneDrive\\Desktop\\Art Folder\\LIKETHEWIND.png")) {}
    if (!clear_texture.loadFromFile("C:\\Users\\yeckr\\OneDrive\\Desktop\\Art Folder\\LIKETHECLEAR.png")) {}
    sf::Sprite wind_sprite(wind_texture);
    sf::Sprite clear_sprite(clear_texture);
    wind_sprite.setPosition(convert_resolution(sf::Vector2f(0, 233)));
    wind_sprite.setScale(.25, .25);
    clear_sprite.setPosition(convert_resolution(sf::Vector2f(7, 678)));
    clear_sprite.setScale(.25, .25);

    // SETTING UP THREADS
    std::vector<std::thread> threads(num_threads);

    sf::Vector2i curr_mouse_pos = sf::Mouse::getPosition(*window);
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
            }
            if (!sf::Mouse::isButtonPressed(sf::Mouse::Left) && event.type == sf::Event::MouseMoved && move_window) {
                // Update Mouse Position
                curr_mouse_pos = sf::Mouse::getPosition(*window);
            }
            // Dragging window around
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && event.type == sf::Event::MouseMoved && move_window) {
                sf::Vector2i mouse_pos = sf::Mouse::getPosition(*window);
                sf::Vector2i change = (curr_mouse_pos - mouse_pos);
                change.x *= zoom_val;
                change.y *= zoom_val;
                view.move(change.x, change.y);
                
                // Move the UI with the user
                ui.move_UI(change);

                // Move other UI Elements
                for (int seg = 0; seg < preview_line.segments; seg++) {
                    (*preview_line.shape)[seg].position = (*preview_line.shape)[seg].position + sf::Vector2f(change);
                }
                for (int seg = 0; seg < preview_curve.segments; seg++) {
                    (*preview_curve.shape)[seg].position = (*preview_curve.shape)[seg].position + sf::Vector2f(change);
                }
                wind_sprite.setPosition(wind_sprite.getPosition() + sf::Vector2f(change));
                clear_sprite.setPosition(clear_sprite.getPosition() + sf::Vector2f(change));
                prev_teleport_min = prev_teleport_min + sf::Vector2f(change.x, change.x);
                prev_tele_fifth_max = prev_tele_fifth_max + sf::Vector2f(change.y, change.y);
                prev_tele_sixth_max = prev_tele_sixth_max + sf::Vector2f(change.y, change.y);
                curr_mouse_pos = mouse_pos;
            }
            if (event.type == sf::Event::MouseButtonPressed and event.type != sf::Event::MouseButtonReleased) {
                sf::Vector2i mouse_pos = sf::Mouse::getPosition(*window);
                sf::Vector2f mouse_conv = window->mapPixelToCoords(mouse_pos);
                sf::Vector2i mouse = sf::Vector2i(mouse_conv.x, mouse_conv.y);

                int eventtype = -1;
                ui.check_collision(UI_render_type, eventtype, mouse, red, green, blue, start_vel_x, start_vel_y, \
                    mass, radius, modifier, particle_amount, temperature, rainbow_mode, viscosity, horizontal_blow, vertical_blow, gravity, \
                    draw_particles, move_window, draw_line, draw_curve, wind_enabled, consume, explode, radioactive, teleportation, particle_swap, iridescent,
                    type);

                // Clears everything from the window
                if (eventtype == 27) {
                    for (int i = 0; i < particles->size(); i++) {
                        delete (*particles)[i];
                    }
                    particles->clear();
                    lines.clear();
                    curves.clear();
                    biggest_radius = 0;
                }
                else if (eventtype == -1) {

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

                            sf::Vector2f position(mouse.x + i, mouse.y + i);

                            sf::Color color;
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
                            for (auto& preview : preview_particles) preview->setFillColor(sf::Color(red, green, blue));

                            Particle* particle = new Particle(radius, position, color, type, mass, velocity, temperature, viscosity, consume, explode, teleportation, particle_swap, iridescent, radioactive);
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
                    zoom_val *= 0.8;
                }
                else if (event.mouseWheelScroll.delta < 0) {
                    view.zoom(1.25f);
                    zoom_val *= 1.25;
                }
            }
        }

        window->setView(view);
        window->clear();

        // UPDATE VALUES
        for (int min_substeps = substeps; min_substeps > 0; min_substeps--) {

            // Boundary to the quadtrees
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
                        particle->particle->setPosition(pos);
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

            // Updating Threads
            int num_particles = particles->size();
            int particles_per_thread = num_particles / num_threads;

            for (int i = 0; i < num_threads; i++) {
                int start_index = i * particles_per_thread;
                int end_index = (i == num_threads - 1) ? num_particles + (num_particles % num_threads) : start_index + particles_per_thread;
                threads[i] = std::thread(update_particles, particles, subdt, gravity, collisions_qt, space_qt, lines, curves);
            }

            // Wait for all threads to finish
            for (auto& thread : threads) {
                thread.join();
            }

            // Chances of particles using their abilities
            for (int step = 0; step < particles->size(); step++) {

                // Generates a random number from 0% - 100% using numbers 0 - 1...
                //      according to frames, so 100% / 100 / fps / substeps = arbitrary fractional percentage.
                float random_number = random_number_generator();
                
                // Swaps random particles if there are more than 2 particles
                if (particles->size() > 1) {
                    
                    // Only swap if a particle is swappable
                    if ((*particles)[step]->swap) {
                        if (random_number * percent_divisor <= swap_chance) {
                            float random_particle = random_number_generator(std::tuple<int, int>(0, int(particles->size() - 1)));
                            if ((*particles)[random_particle]->swap) {
                                sf::Vector2f pos1 =  (*particles)[step]->particle->getPosition();
                                sf::Vector2f* vel1 = (*particles)[step]->velocity;
                                sf::Vector2f pos2 =  (*particles)[random_particle]->particle->getPosition();
                                sf::Vector2f* vel2 = (*particles)[random_particle]->velocity;
                                (*particles)[step]->particle->setPosition(pos2);
                                (*particles)[random_particle]->particle->setPosition(pos1);
                                (*particles)[step]->velocity = vel2;
                                (*particles)[random_particle]->velocity = vel1;
                            }
                        }
                    }
                }

                // Teleport Particles
                if ((*particles)[step]->teleportation) {
                    if (random_number * percent_divisor <= teleport_chance) {
                        float random_x = random_number_generator(std::tuple<int, int>(0, int(windowsize.x)));
                        float random_y = random_number_generator(std::tuple<int, int>(0, int(windowsize.y)));
                        (*particles)[step]->particle->setPosition(random_x, random_y);
                    }
                }

                // Iridescence Chance
                if ((*particles)[step]->iridescent) {
                    if (random_number * percent_divisor <= iridescent_chance) {
                        // Chooses to shift red, green, or blue
                        float color_choice = random_number_generator(std::tuple<int, int>(1, 3));
                        float color_shift =  random_number_generator(std::tuple<int, int>(-35, 35));
                        sf::Color part_color = (*particles)[step]->particle->getFillColor();
                        if (color_choice == 1) {
                            if      (part_color.r + color_shift < 0) color_shift = abs(color_shift);
                            else if (part_color.r + color_shift > 255) color_shift = -color_shift;
                            part_color.r += color_shift;
                        }
                        else if (color_choice == 2) {
                            if      (part_color.g + color_shift < 0) color_shift = abs(color_shift);
                            else if (part_color.g + color_shift > 255) color_shift = -color_shift;
                            part_color.g += color_shift;
                        }
                        else if (color_choice == 3) {
                            if      (part_color.b + color_shift < 0) color_shift = abs(color_shift);
                            else if (part_color.b + color_shift > 255) color_shift = -color_shift;
                            part_color.b += color_shift;
                        }
                        (*particles)[step]->particle->setFillColor(part_color);
                    }
                }
            }

            // Effects Preview Particles
            for (int i = 2; i < 11; i++) {
                
                float random_number = random_number_generator();
                float the_chosen_one = random_number_generator(std::tuple<int, int>(1, 3));
                // Update consume particle
                if (i == 2 || (the_chosen_one == 1 && i == 5)) {
                    if (random_number * percent_divisor <= teleport_chance) {
                        float radius = preview_particles[i]->getRadius();
                        if (grow && i != 5) {
                            radius++;
                            if (radius >= 9) grow = false;
                        }
                        else if (!grow && i != 5) {
                            radius--;
                            if (radius <= 3) grow = true;
                        }
                        else if (grow_rad && i != 2) {
                            radius++;
                            if (radius >= 9) grow_rad = false;
                        }
                        else {
                            radius--;
                            if (radius <= 3) grow_rad = true;
                        }
                        preview_particles[i]->setRadius(radius);
                        preview_particles[i]->setOrigin(sf::Vector2f(radius, radius));
                    }
                }
                // Update explode particle
                else if (i == 3) {

                }
                // Update negative particle
                else if (i == 4) {

                }
                // Update teleport particle
                else if (i == 6 || (the_chosen_one == 2 && i == 5)) {
                    if (random_number * percent_divisor <= teleport_chance) {
                        sf::Vector2f prev_tele;
                        if (i == 5) {
                            prev_tele = prev_tele_fifth_max;
                        }
                        if (i == 6) {
                            prev_tele = prev_tele_sixth_max;
                        }
                        float random_x = random_number_generator(std::tuple<int, int>(prev_teleport_min.x, prev_teleport_min.y));
                        float random_y = random_number_generator(std::tuple<int, int>(prev_tele.x, prev_tele.y));
                        preview_particles[i]->setPosition(random_x, random_y);
                    }
                }
                // Update swap particles | indecies 7 & 8
                else if (i == 7) {
                    if (random_number * percent_divisor <= swap_chance) {
                        float random_particle = random_number_generator(std::tuple<int, int>(0, int(particles->size() - 1)));
                        sf::Vector2f pos1 = preview_particles[i]->getPosition();
                        sf::Vector2f pos2 = preview_particles[i+1]->getPosition();
                        preview_particles[i]->setPosition(pos2);
                        preview_particles[i+1]->setPosition(pos1);
                    }
                }
                // Update iridescent particle
                else if (i == 9 || (the_chosen_one == 3 && i == 5)) {
                    if (random_number * percent_divisor <= iridescent_chance) {
                        // Chooses to shift red, green, or blue
                        float color_choice = random_number_generator(std::tuple<int, int>(1, 3));
                        float color_shift = random_number_generator(std::tuple<int, int>(-35, 35));
                        sf::Color part_color = preview_particles[i]->getFillColor();
                        if (color_choice == 1) {
                            if (part_color.r + color_shift < 0) color_shift = abs(color_shift);
                            else if (part_color.r + color_shift > 255) color_shift = -color_shift;
                            part_color.r += color_shift;
                        }
                        else if (color_choice == 2) {
                            if (part_color.g + color_shift < 0) color_shift = abs(color_shift);
                            else if (part_color.g + color_shift > 255) color_shift = -color_shift;
                            part_color.g += color_shift;
                        }
                        else if (color_choice == 3) {
                            if (part_color.b + color_shift < 0) color_shift = abs(color_shift);
                            else if (part_color.b + color_shift > 255) color_shift = -color_shift;
                            part_color.b += color_shift;
                        }
                        preview_particles[i]->setFillColor(part_color);
                    }
                }
                // Update fire particle
                else if (i == 10) {
                    if (random_number * percent_divisor <= teleport_chance) {
                        if (heating) {
                            sf::Color temp_color = fire_color_updater(preview_temp);
                            preview_particles[i]->setFillColor(temp_color);
                            if (preview_temp >= 250) heating = false;
                            preview_temp++;
                        }
                        else {
                            sf::Color temp_color = fire_color_updater(preview_temp);
                            preview_particles[i]->setFillColor(temp_color);
                            if (preview_temp <= 0) heating = true;
                            preview_temp--;
                        }
                    }
                }
            }

            // Wind Physics
            if (wind_enabled) wind_sim(particles, horizontal_blow, vertical_blow);

            delete collisions_qt;
            delete space_qt;
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

        // Draws the particle UI
        for (int object = 0; object < UI_vectors.size(); object++) {
            for (int rectangle = 0; rectangle < UI_vectors[object].size(); rectangle++) {
                window->draw((*UI_vectors[object][rectangle]));
            }

            // Draws Preview Items
            for (auto& preview : preview_particles) {
                window->draw(*preview);
            }
            window->draw(*preview_line.shape);
            window->draw(*preview_curve.shape);
            window->draw(wind_sprite);
            window->draw(clear_sprite);
        }

        // Draws the text on top of UI
        for (int text = 0; text < texts.size(); text++) {
            window->draw(*texts.at(text));
        }

        window->display();
    }
    return 0;
}

/*
TODO LIST

Bop it tells you to cry about it :>
Make UI follow the player
*/