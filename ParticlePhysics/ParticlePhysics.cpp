#include <iostream>
#include <SFML/Graphics.hpp>
#include "particle.h"
#include <cmath>
#include <sstream>
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
void space_update_position() {
    // Simulates space physics
}
void regular_update_position(Particle* particle, sf::CircleShape* shape, int index, float deltaTime) {
    
    // Move particle so far along the given path.
    float gravity = 9.81f;
    float x = shape->getPosition().x;
    float y = shape->getPosition().y;

    particle->velocity->y += deltaTime * gravity * particle->mass;
    shape->setPosition(x + particle->velocity->x * deltaTime, y + particle->velocity->y * deltaTime);
}
void update_particle(std::vector<Particle*> particles, int index, float deltaTime) {
    regular_update_position(particles[index], particles[index]->particle, index, deltaTime);
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
    vectors.push_back(std::vector<sf::RectangleShape>()); // Red Color Channel               [2]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Green Color Channel             [3]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Blue Color Channel              [4]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Box to Change Radius            [5]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Box to Change Mass              [6]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Boxes to Change Velocity        [7]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Boxes to Change # of Particles  [8]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Boxes to Change Multipler       [9]

    // Declaring all the necessary colors
    sf::Color light_grey(75, 75, 75);
    sf::Color dark_grey(50, 50, 50);
    sf::Color purple(139, 68, 219);
    sf::Color pastel_orange(194, 130, 93);
    sf::Color pastel_yellow(240, 233, 137);
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
    sf::Vector2f bg = convert_resolution(sf::Vector2f(450, 450));
    sf::Vector2f hbm = convert_resolution(sf::Vector2f(450, 7));
    sf::Vector2f sb = convert_resolution(sf::Vector2f(7, 450));

    // General Box Environment
    sf::RectangleShape background = make_bar(bg, dark_grey, convert_resolution(sf::Vector2f(1920, 0)));
    sf::RectangleShape topbar =     make_bar(hbm, light_grey, convert_resolution(sf::Vector2f(1920, 0)));
    sf::RectangleShape bottombar =  make_bar(hbm, light_grey, convert_resolution(sf::Vector2f(1920, 450)));
    sf::RectangleShape sidebar =    make_bar(sb, light_grey, convert_resolution(sf::Vector2f(1920, 0)));

    // General Button Sizes
    sf::Vector2f plusminus =  convert_resolution(sf::Vector2f(20, 40));
    sf::Vector2f colorbox = convert_resolution(sf::Vector2f(40, 40));
    
    // RGB Buttons
    sf::RectangleShape red =    make_bar(colorbox,  pastel_red,   convert_resolution(sf::Vector2f(1920 + 75,  125)));
    sf::RectangleShape rplus =  make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 115,  125)));
    sf::RectangleShape rminus = make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 55, 125)));
    sf::RectangleShape green =  make_bar(colorbox,  pastel_green, convert_resolution(sf::Vector2f(1920 + 190, 125)));
    sf::RectangleShape gplus =  make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 230, 125)));
    sf::RectangleShape gminus = make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 170, 125)));
    sf::RectangleShape blue =   make_bar(colorbox,  pastel_blue,  convert_resolution(sf::Vector2f(1920 + 305, 125)));
    sf::RectangleShape bplus =  make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 345, 125)));
    sf::RectangleShape bminus = make_bar(plusminus, light_grey,   convert_resolution(sf::Vector2f(1920 + 285, 125)));

    // Radius, Size, and Velocity
    sf::RectangleShape radius =    make_bar(colorbox, pastel_orange, convert_resolution(sf::Vector2f(1920 + 132, 200)));
    sf::RectangleShape radplus =   make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 172, 200)));
    sf::RectangleShape radminus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 112, 200)));
    sf::RectangleShape mass =      make_bar(colorbox, pastel_yellow, convert_resolution(sf::Vector2f(1920 + 247, 200)));
    sf::RectangleShape massplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 287, 200)));
    sf::RectangleShape massminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 227, 200)));
    sf::RectangleShape velocityx = make_bar(colorbox, purple, convert_resolution(sf::Vector2f(1920 + 132, 260)));
    sf::RectangleShape velxplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 172, 260)));
    sf::RectangleShape velxminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 112, 260)));
    sf::RectangleShape velocityy = make_bar(colorbox, purple, convert_resolution(sf::Vector2f(1920 + 247, 260)));
    sf::RectangleShape velyplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 287, 260)));
    sf::RectangleShape velyminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 227, 260)));

    // Number of Particles to Spawn in
    sf::RectangleShape amount = make_bar(colorbox, pastel_orange, convert_resolution(sf::Vector2f(1920 + 132, 320)));
    sf::RectangleShape amtplus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 172, 320)));
    sf::RectangleShape amtminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 112, 320)));

    // Multiplier for variable adjustments
    sf::RectangleShape multiplier = make_bar(colorbox, pastel_yellow, convert_resolution(sf::Vector2f(1920 + 247, 320)));
    sf::RectangleShape multiplus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 287, 320)));
    sf::RectangleShape multiminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 227, 320)));

    vectors[0].push_back(rect4);
    vectors[0].push_back(rect1);
    vectors[0].push_back(rect2);
    vectors[0].push_back(rect3);
    vectors[1].push_back(background);
    vectors[1].push_back(topbar);
    vectors[1].push_back(bottombar);
    vectors[1].push_back(sidebar);
    vectors[2].push_back(rminus);
    vectors[2].push_back(red);
    vectors[2].push_back(rplus);
    vectors[3].push_back(gminus);
    vectors[3].push_back(green);
    vectors[3].push_back(gplus);
    vectors[4].push_back(bminus);
    vectors[4].push_back(blue);
    vectors[4].push_back(bplus);
    vectors[5].push_back(radminus);
    vectors[5].push_back(radius);
    vectors[5].push_back(radplus);
    vectors[6].push_back(massminus);
    vectors[6].push_back(mass);
    vectors[6].push_back(massplus);
    vectors[7].push_back(velxminus);
    vectors[7].push_back(velocityx);
    vectors[7].push_back(velxplus);
    vectors[7].push_back(velyminus);
    vectors[7].push_back(velocityy);
    vectors[7].push_back(velyplus);
    vectors[8].push_back(amtminus);
    vectors[8].push_back(amount);
    vectors[8].push_back(amtplus);
    vectors[9].push_back(multiminus);
    vectors[9].push_back(multiplier);
    vectors[9].push_back(multiplus);
    return vectors;
}
bool mouse_collide(sf::Vector2i mouse, sf::Vector2f position, sf::Vector2f size) {
    if (mouse.x > position.x && mouse.x < position.x + size.x && mouse.y > position.y && mouse.y < position.y + size.y) return true;
    return false;
}
void custom_message(sf::Text* message, sf::Vector2f position) {
    message->setCharacterSize(15);
    message->setFillColor(sf::Color::Black);
    message->setPosition(position);
}
int main()
{
    int fps = 165;
    sf::RenderWindow window(sf::VideoMode(windowsize.x, windowsize.y), "My Life is in Constant Torment :)");
    window.setFramerateLimit(fps);
 
    std::string UI_render_type = "closed";
    std::vector<std::vector<sf::RectangleShape>> UI_vectors = create_UI(window, UI_render_type);

    std::vector<Particle*> particles;
    std::vector<sf::Text*> texts;
    sf::Font font;
    if (!font.loadFromFile("C:\\WINDOWS\\FONTS\\ARIAL.TTF")) {}

    std::vector<sf::Vector3f> positions;
    for (int i = 0; i < particles.size(); i++) {
        positions.push_back(sf::Vector3f(particles[i]->particle->getPosition().x, particles[i]->particle->getPosition().y, particles[i]->radius));
    }

    // For perforance crap
    //kdt::KDTree<sf::Vector3f> kdtree(positions);

    // Standard particle features
    int radius = 5;
    int mass = 150;
    int start_vel_x = 0;
    int start_vel_y = 0;
    int modifier = 1;
    int particle_amount = 1;
    int red = 255;
    int green = 0;
    int blue = 0;

    sf::Text* velxstring =      new sf::Text(std::to_string(start_vel_x), font);
    sf::Text* velystring =      new sf::Text(std::to_string(start_vel_y), font);
    sf::Text* masstring =       new sf::Text(std::to_string(mass), font);
    sf::Text* radiustring =     new sf::Text(std::to_string(radius), font);
    sf::Text* modistring =      new sf::Text(std::to_string(modifier), font);
    sf::Text* part_amt_string = new sf::Text(std::to_string(particle_amount), font);
    sf::Text* red_string =      new sf::Text(std::to_string(red), font);
    sf::Text* green_string =    new sf::Text(std::to_string(green), font);
    sf::Text* blue_string =     new sf::Text(std::to_string(blue), font);

    custom_message(velxstring,      convert_resolution(sf::Vector2f(1920 + 132, 265)));
    custom_message(velystring,      convert_resolution(sf::Vector2f(1920 + 247, 265)));
    custom_message(masstring,       convert_resolution(sf::Vector2f(1920 + 247, 205)));
    custom_message(radiustring,     convert_resolution(sf::Vector2f(1920 + 132, 205)));
    custom_message(modistring,      convert_resolution(sf::Vector2f(1920 + 247, 325)));
    custom_message(part_amt_string, convert_resolution(sf::Vector2f(1920 + 132, 325)));
    custom_message(red_string,      convert_resolution(sf::Vector2f(1920 + 75, 130)));
    custom_message(green_string,    convert_resolution(sf::Vector2f(1920 + 190, 130)));
    custom_message(blue_string,     convert_resolution(sf::Vector2f(1920 + 305, 130)));

    texts.push_back(red_string);
    texts.push_back(green_string);
    texts.push_back(blue_string);
    texts.push_back(radiustring);
    texts.push_back(masstring);
    texts.push_back(velxstring);
    texts.push_back(velystring);
    texts.push_back(part_amt_string);
    texts.push_back(modistring);

    float deltaTime = 1.f/fps;
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
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
                else if (event.key.code == sf::Keyboard::Delete) {
                    particles.clear();
                }
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mouse = sf::Mouse::getPosition(window);
                
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
                                    if (red > 256) { red = 256; }
                                    if (red < 0) { red = 0; }
                                    texts[0]->setString(std::to_string(red));
                                    eventtype = 2;
                                }
                            }
                            else if (i == 3) {
                                // Adjusting Green
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { green -= modifier; }
                                    else if (j == 2) { green += modifier; }
                                    if (green > 256) { green = 256; }
                                    if (green < 0) { green = 0; }
                                    texts[1]->setString(std::to_string(green));
                                    eventtype == 3;
                                }
                            }
                            else if (i == 4) {
                                // Adjusting Blue
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { blue -= modifier; }
                                    else if (j == 2) { blue += modifier; }
                                    if (blue > 256) { blue = 256; }
                                    if (blue < 0) { blue = 0; }
                                    texts[2]->setString(std::to_string(blue));
                                    eventtype == 4;
                                }
                            }
                            else if (i == 5) {
                                // Adjusting The Radius
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0 && radius > 1) { radius -= modifier; }
                                    else if (j == 2 && radius < windowsize.y) { radius += modifier; }
                                    if (radius < 1) { radius = 1; }
                                    if (radius > windowsize.y) { radius = windowsize.y; }
                                    texts[3]->setString(std::to_string(radius));
                                    eventtype == 5;
                                }
                            }
                            else if (i == 6) {
                                // Adjusting The Mass
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { mass -= modifier; }
                                    else if (j == 2) { mass += modifier; }
                                    if (mass < 1) { mass = 1; }
                                    texts[4]->setString(std::to_string(mass));
                                    eventtype == 6;
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
                                    eventtype == 7;
                                }
                            }
                            else if (i == 8) {
                                // Adjusting The Number of Particles to Spawn
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { particle_amount -= modifier; }
                                    else if (j == 2) { particle_amount += modifier; }
                                    if (particle_amount < 1) { particle_amount = 1; }
                                    texts[7]->setString(std::to_string(particle_amount));
                                    eventtype == 8;
                                }
                            }
                            else if (i == 9) {
                                // Adjusting The Number of Particles to Spawn
                                if (mouse_collide(mouse, pos, size)) {
                                    if (j == 0) { modifier--; }
                                    else if (j == 2) { modifier++; }
                                    if (modifier < 1) { modifier = 1; }
                                    texts[8]->setString(std::to_string(modifier));
                                    eventtype == 9;
                                }
                            }
                        }
                    }
                }
                
                if (eventtype == -1) {
                    // Placing particle
                    for (int i = 0; i < particle_amount; i++) {
                        sf::Vector2f position(mouse.x + 15 * i, mouse.y);
                        // RAINBOW POWER!!!!!11!11!!
                        //sf::Color color = color_getter(red, green, blue, r_dir, g_dir, b_dir);
                        sf::Color color(red, green, blue);
                        int grav_type = 1;
                        sf::Vector2f* velocity = new sf::Vector2f(start_vel_x, start_vel_y);

                        Particle* particle = new Particle(radius, position, color, grav_type, mass, velocity);
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

        // Draws the text on top of UI
        for (int text = 0; text < texts.size(); text++) {
            window.draw(*texts.at(text));
            std::cout << text << std::endl;
        }

        window.display();
    }

    return 0;
}