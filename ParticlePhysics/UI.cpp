#include "UI.h"
#include <iostream>

UserInterface::UserInterface(sf::Vector2f ws) {
    windowsize = ws;
    if (!font.loadFromFile("C:\\WINDOWS\\FONTS\\ARIAL.TTF")) {}
}
bool mouse_collide(sf::Vector2i mouse, sf::Vector2f position, sf::Vector2f size) {
    if (mouse.x > position.x && mouse.x < position.x + size.x && mouse.y > position.y && mouse.y < position.y + size.y) return true;
    return false;
}
float UserInterface::convert_x(float x) {
    return (windowsize.x * x / 1920);
}
float UserInterface::convert_y(float y) {
    return (windowsize.y * y / 1080);
}
sf::Vector2f UserInterface::convert_resolution(sf::Vector2f coordinates) {
    sf::Vector2f res(1920, 1080);
    float x = convert_x(coordinates.x);
    float y = convert_y(coordinates.y);
    return sf::Vector2f(x, y);
}
sf::RectangleShape* UserInterface::make_bar(sf::Vector2f size, sf::Color color, sf::Vector2f position) {
    sf::RectangleShape* rectangle = new sf::RectangleShape(size);
    rectangle->setFillColor(color);
    rectangle->setPosition(position);
    return rectangle;
}
void custom_message(sf::Text* message, sf::Vector2f position) {
    message->setCharacterSize(15);
    message->setFillColor(sf::Color::Black);
    message->setPosition(position);
}
void UserInterface::move_UI(sf::Vector2i move) {
    // Moves the UI based on a Vector2i
    for (int object = 0; object < vectors.size(); object++) {
        for (int rectangle = 0; rectangle < vectors[object].size(); rectangle++) {
            sf::Vector2f pos = vectors[object][rectangle]->getPosition();
            vectors[object][rectangle]->setPosition(pos.x + move.x, pos.y + move.y);
        }
    }
    for (auto& prev_part : preview_particles) {
        sf::Vector2f pos = prev_part->getPosition();
        prev_part->setPosition(pos.x + move.x, pos.y + move.y);
    }
    for (int object = 0; object < texts.size(); object++) {
        sf::Vector2f pos = texts[object]->getPosition();
        texts[object]->setPosition(pos.x + move.x, pos.y + move.y);
    }
}
void UserInterface::check_collision(std::string& UI_render_type, int& eventtype, sf::Vector2i mouse, int& red, int& green, int& blue, int& vel_x, int& vel_y,
    int& mass, int& radius, int& modifier, int& particle_amount, int& temperature, bool& rainbow_mode, float& viscosity, float& h_blow, float& v_blow, float& gravity,
    bool& draw_particles, bool& move_window, bool& draw_line, bool& draw_curve, bool& wind_enabled, bool& consume, bool& explode, bool& radioactive, bool& teleportation, bool& particle_swap,
    bool& iridescent, std::string& type) {

    sf::Color inactive(75, 75, 75);
    sf::Color active(75, 150, 75);

    int shift = 0;

    if (UI_render_type == "closed") shift = -400;
    else shift = 400;

    for (int i = 0; i < vectors.size(); i++) {

        // Check if they are wanting to close the window
        for (int j = 0; j < vectors[i].size(); j++) {
            sf::Vector2f size = vectors[i][j]->getSize();
            sf::Vector2f pos = vectors[i][j]->getPosition();
            if (mouse_collide(mouse, pos, size)) {
                if (i == 0) {
                    UI_render_type = (UI_render_type == "closed") ? "open" : "closed";
                    for (int object = 0; object < 14; object++) {
                        for (int rectangle = 0; rectangle < vectors[object].size(); rectangle++) {
                            sf::Vector2f pos = vectors[object][rectangle]->getPosition();
                            vectors[object][rectangle]->setPosition(pos.x + convert_x(shift), pos.y);
                        }
                    }
                    for (int object = 0; object < texts.size(); object++) {
                        sf::Vector2f pos = texts[object]->getPosition();
                        texts[object]->setPosition(pos.x + convert_x(shift), pos.y);
                    }
                    sf::Vector2f pp_pos = preview_particles[0]->getPosition();
                    preview_particles[0]->setPosition(pp_pos.x + convert_x(shift), pp_pos.y);
                    eventtype = i;
                }
                else if (i == 1) {
                    // Colliding with background | Nothing Happens
                }
                else if (i == 2) {
                    // Adjusting Red
                    if (j == 0) { red -= modifier; }
                    else if (j == 2) { red += modifier; }
                    if (red > 255) { red = 255; }
                    if (red < 0) { red = 0; }
                    for (auto& preview : preview_particles) preview->setFillColor(sf::Color(red, green, blue));
                    texts[0]->setString(std::to_string(red));
                }
                else if (i == 3) {
                    // Adjusting Green
                    if (j == 0) { green -= modifier; }
                    else if (j == 2) { green += modifier; }
                    if (green > 255) { green = 255; }
                    if (green < 0) { green = 0; }
                    for (auto& preview : preview_particles) preview->setFillColor(sf::Color(red, green, blue));
                    texts[1]->setString(std::to_string(green));
                }
                else if (i == 4) {
                    // Adjusting Blue
                    if (j == 0) { blue -= modifier; }
                    else if (j == 2) { blue += modifier; }
                    if (blue > 255) { blue = 255; }
                    if (blue < 0) { blue = 0; }
                    for (auto& preview : preview_particles) preview->setFillColor(sf::Color(red, green, blue));
                    texts[2]->setString(std::to_string(blue));
                }
                else if (i == 5) {
                    // Adjusting The Radius
                    if (j == 0 && radius > 1) { radius -= modifier; }
                    else if (j == 2) { radius += modifier; }
                    if (radius < 1) { radius = 1; }
                    if (radius > windowsize.y / 2) { radius = windowsize.y / 2; }
                    preview_particles[0]->setRadius(radius);
                    preview_particles[0]->setPosition(convert_resolution(sf::Vector2f(1730 - radius, 70 - radius)));
                    texts[3]->setString(std::to_string(radius));
                }
                else if (i == 6) {
                    // Adjusting The Mass
                    if (j == 0) { mass -= modifier; }
                    else if (j == 2) { mass += modifier; }
                    if (mass < 1) { mass = 1; }
                    texts[4]->setString(std::to_string(mass));
                }
                else if (i == 7) {
                    // Adjusting The Velocity
                    if (j == 0) { vel_x -= modifier; }
                    else if (j == 2) { vel_x += modifier; }
                    else if (j == 3) { vel_y -= modifier; }
                    else if (j == 5) { vel_y += modifier; }
                    texts[5]->setString(std::to_string(vel_x));
                    texts[6]->setString(std::to_string(vel_y));
                }
                else if (i == 8) {
                    // Adjusting The Number of Particles to Spawn
                    if (j == 0) { particle_amount -= modifier; }
                    else if (j == 2) { particle_amount += modifier; }
                    if (particle_amount < 1) { particle_amount = 1; }
                    texts[7]->setString(std::to_string(particle_amount));
                }
                else if (i == 9) {
                    // Adjusting The Number of Particles to Spawn
                    if (j == 0) { modifier--; }
                    else if (j == 2) { modifier++; }
                    if (modifier < 1) { modifier = 1; }
                    texts[8]->setString(std::to_string(modifier));
                }
                else if (i == 10) {
                    // Adjusting The Number of Particles to Spawn
                    if (j == 0) { gravity -= .01 * modifier; }
                    else if (j == 2) { gravity += .01 * modifier; }
                    if (gravity < 0) { gravity = 0; }
                    texts[9]->setString(std::to_string(gravity));
                }
                else if (i == 11) {
                    // Adjusting The Number of Particles to Spawn
                    if (j == 0) {
                        if (rainbow_mode) {
                            rainbow_mode = false;
                            vectors[11][j]->setFillColor(sf::Color(215, 215, 185));
                        }
                        else {
                            rainbow_mode = true;
                            vectors[11][j]->setFillColor(sf::Color(215, 215, 0));
                        }
                    }
                }
                else if (i == 12) {
                    // Adjusting The Particle's Temperature
                    if (j == 0) { temperature -= modifier; }
                    else if (j == 2) { temperature += modifier; }
                    if (temperature < 0) temperature = 0;
                    texts[10]->setString(std::to_string(temperature));
                }
                else if (i == 13) {
                    // Adjusting the Wind values
                    if (j == 0) { h_blow--; }
                    if (j == 2) { h_blow++; }
                    if (j == 3) { v_blow--; }
                    if (j == 5) { v_blow++; }
                }
                else if (i == 14) {
                    // Moving Window
                    draw_line = false;
                    draw_curve = false;
                    draw_particles = false;
                    move_window = true;
                    vectors[i][1]->setFillColor(active);
                    vectors[i + 1][1]->setFillColor(inactive);
                    vectors[i + 2][1]->setFillColor(inactive);
                    vectors[i + 3][1]->setFillColor(inactive);
                }
                else if (i == 15) {
                    // Drawing Particles
                    draw_line = false;
                    draw_curve = false;
                    draw_particles = true;
                    move_window = false;

                    vectors[i - 1][1]->setFillColor(inactive);
                    vectors[i][1]->setFillColor(active);
                    vectors[i + 1][1]->setFillColor(inactive);
                    vectors[i + 2][1]->setFillColor(inactive);
                }
                else if (i == 16) {
                    // Drawing Lines
                    draw_line = true;
                    draw_curve = false;
                    draw_particles = false;
                    move_window = false;

                    vectors[i - 2][1]->setFillColor(inactive);
                    vectors[i - 1][1]->setFillColor(inactive);
                    vectors[i][1]->setFillColor(active);
                    vectors[i + 1][1]->setFillColor(inactive);
                }
                else if (i == 17) {
                    // Drawing Curves
                    draw_line = false;
                    draw_curve = true;
                    draw_particles = false;
                    move_window = false;
                    eventtype = 16;

                    vectors[i - 3][1]->setFillColor(inactive);
                    vectors[i - 2][1]->setFillColor(inactive);
                    vectors[i - 1][1]->setFillColor(inactive);
                    vectors[i][1]->setFillColor(active);
                }
                else if (i == 18) {
                    // Enable Wind
                    wind_enabled = wind_enabled ? false : true;
                    if (wind_enabled) vectors[i][1]->setFillColor(active);
                    else vectors[i][1]->setFillColor(inactive);
                }
                else if (i == 19) {
                    // Enable Consumption | Particles with this property can "eat" other particles regardless of other properties.
                    consume = consume ? false : true;
                    if (consume) vectors[i][1]->setFillColor(active);
                    else vectors[i][1]->setFillColor(inactive);
                }
                else if (i == 20) {
                    // Enable Explosion | Particles explode into an undetermined amount of particles on contact if fast enough
                    explode = explode ? false : true;
                    if (explode) vectors[i][1]->setFillColor(active);
                    else vectors[i][1]->setFillColor(inactive);
                }
                else if (i == 21) {
                    // Enable Negative Mass
                    mass = -mass;
                    if (mass < 0) vectors[i][1]->setFillColor(active);
                    else vectors[i][1]->setFillColor(inactive);
                }
                else if (i == 22) {
                    // Enable Radioactivity | Explained in chernobyl_particle()
                    radioactive = radioactive ? false : true;
                    if (radioactive) vectors[i][1]->setFillColor(active);
                    else vectors[i][1]->setFillColor(inactive);
                }
                else if (i == 23) {
                    // Enable Teleportation | Spontaneously teleports at random
                    teleportation = teleportation ? false : true;
                    if (teleportation) vectors[i][1]->setFillColor(active);
                    else vectors[i][1]->setFillColor(inactive);
                }
                else if (i == 24) {
                    // Enable Swapping  | Particles with this tag can swap with other particles that contain the particle_swap tag
                    particle_swap = particle_swap ? false : true;
                    if (particle_swap) vectors[i][1]->setFillColor(active);
                    else vectors[i][1]->setFillColor(inactive);
                }
                else if (i == 25) {
                    // Enable Iridescence | Spontaneously changes colors at random
                    iridescent = iridescent ? false : true;
                    if (iridescent) vectors[i][1]->setFillColor(active);
                    else vectors[i][1]->setFillColor(inactive);
                }
                else if (i == 26) {
                    // Enable Fire
                    type = (type == "fire") ? "normal" : "fire";
                    if (type == "fire") vectors[i][1]->setFillColor(active);
                    else vectors[i][1]->setFillColor(inactive);
                }
                else if (i == 27) {
                    // Clear Screen
                }
                eventtype = i;
                break;
            }
        }
    }

    // Eventtype changed, it's colliding with the UI
    return;
}
void UserInterface::PictureDisplay() {

    // Particles for Preview Buttons
    sf::CircleShape* color_preview = new sf::CircleShape();
    sf::CircleShape* draw_particle_preview = new sf::CircleShape();
    sf::CircleShape* consume_preview = new sf::CircleShape();
    sf::CircleShape* explode_preview = new sf::CircleShape();
    sf::CircleShape* negative_mass_preview = new sf::CircleShape();
    sf::CircleShape* radioactive_preview = new sf::CircleShape();
    sf::CircleShape* teleport_preview = new sf::CircleShape();
    sf::CircleShape* swap1_preview = new sf::CircleShape();
    sf::CircleShape* swap2_preview = new sf::CircleShape();
    sf::CircleShape* iridescent_preview = new sf::CircleShape();
    sf::CircleShape* fire_preview = new sf::CircleShape();

    color_preview->setRadius(5);         //  [0]
    draw_particle_preview->setRadius(5); //  [1]
    consume_preview->setRadius(5);       //  [2]
    explode_preview->setRadius(5);       //  [3]
    negative_mass_preview->setRadius(5); //  [4]
    radioactive_preview->setRadius(5);   //  [5]
    teleport_preview->setRadius(5);      //  [6]
    swap1_preview->setRadius(5);         //  [7]
    swap2_preview->setRadius(2);         //  [8]
    iridescent_preview->setRadius(5);    //  [9]
    fire_preview->setRadius(5);          // [10]

    color_preview->setOrigin(sf::Vector2f(5, 5));
    draw_particle_preview->setOrigin(sf::Vector2f(5, 5));
    consume_preview->setOrigin(sf::Vector2f(5, 5));
    explode_preview->setOrigin(sf::Vector2f(5, 5));
    negative_mass_preview->setOrigin(sf::Vector2f(5, 5));
    radioactive_preview->setOrigin(sf::Vector2f(5, 5));
    teleport_preview->setOrigin(sf::Vector2f(5, 5));
    swap1_preview->setOrigin(sf::Vector2f(5, 5));
    swap2_preview->setOrigin(sf::Vector2f(2, 2));
    iridescent_preview->setOrigin(sf::Vector2f(5, 5));
    fire_preview->setOrigin(sf::Vector2f(5, 5));

    color_preview->setFillColor(sf::Color(255, 0, 0));
    draw_particle_preview->setFillColor(sf::Color(255, 0, 0));
    consume_preview->setFillColor(sf::Color(255, 0, 0));
    explode_preview->setFillColor(sf::Color(255, 0, 0));
    negative_mass_preview->setFillColor(sf::Color(255, 0, 0));
    radioactive_preview->setFillColor(sf::Color(255, 0, 0));
    teleport_preview->setFillColor(sf::Color(255, 0, 0));
    swap1_preview->setFillColor(sf::Color(255, 0, 0));
    swap2_preview->setFillColor(sf::Color(255, 0, 0));
    iridescent_preview->setFillColor(sf::Color(255, 0, 0));
    fire_preview->setFillColor(sf::Color(255, 0, 0));

    color_preview->setPosition(        convert_resolution(sf::Vector2f(1920 + 210 - 5, 70 - 5)));
    draw_particle_preview->setPosition(convert_resolution(sf::Vector2f(24,  94)));
    consume_preview->setPosition(      convert_resolution(sf::Vector2f(24, 294)));
    explode_preview->setPosition(      convert_resolution(sf::Vector2f(24, 344)));
    negative_mass_preview->setPosition(convert_resolution(sf::Vector2f(24, 394)));
    radioactive_preview->setPosition(  convert_resolution(sf::Vector2f(24, 444)));
    teleport_preview->setPosition(     convert_resolution(sf::Vector2f(24, 494)));
    swap1_preview->setPosition(        convert_resolution(sf::Vector2f(16, 550)));
    swap2_preview->setPosition(        convert_resolution(sf::Vector2f(35, 537)));
    iridescent_preview->setPosition(   convert_resolution(sf::Vector2f(24, 594)));
    fire_preview->setPosition(         convert_resolution(sf::Vector2f(24, 644)));

    // PARTICLE AND ENVIRONMENT MANIPULATION
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Open/Close Button               [0]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Main background display for UI  [1]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Red Color Channel               [2]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Green Color Channel             [3]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Blue Color Channel              [4]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Box to Change Radius            [5]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Box to Change Mass              [6]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Boxes to Change Velocity        [7]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Boxes to Change # of Particles  [8]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Boxes to Change Multipler       [9]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Boxes to Change Gravity        [10]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Box to toggle rainbow mode     [11]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Box to toggle temperature      [12]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Box to change wind values      [13]

    // WHAT THE USER WANTS TO DO
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Move Window                    [14] 
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Draw Particles                 [15]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Draw Curves                    [16]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Draw Lines                     [17]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Toggle Wind                    [18]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Toggle Consumption             [19]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Toggle Explosion               [20]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Toggle -Mass                   [21]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Toggle Radioactivity           [22]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Toggle Teleportation           [23]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Toggle Swapping                [24]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Toggle Iridescence             [25]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Toggle Fire                    [26]
    vectors.push_back(std::vector<sf::RectangleShape*>()); // Clear Screen                   [27]

    // Declaring all the necessary colors
    sf::Color light_grey(75, 75, 75);
    sf::Color dark_grey(50, 50, 50);
    sf::Color purple(139, 68, 219);
    sf::Color pastel_orange(194, 130, 93);
    sf::Color pastel_yellow(240, 233, 137);
    sf::Color pastel_red(196, 68, 61);
    sf::Color pastel_green(106, 204, 92);
    sf::Color pastel_blue(94, 89, 194);
    sf::Color white(215, 215, 185);
    sf::Color dark_blue(5, 13, 64);
    sf::Color nice_green(75, 150, 75);

    // Bars surrounding the Open/Close Button
    sf::Vector2f hb = convert_resolution(sf::Vector2f(50, 7));
    sf::Vector2f vb = convert_resolution(sf::Vector2f(7, 75));
    sf::Vector2f backdrop = convert_resolution(sf::Vector2f(50, 75));

    // Open/Close Button
    sf::RectangleShape* rect1 = make_bar(hb, light_grey, convert_resolution(sf::Vector2f(1920 - 50, 75)));
    sf::RectangleShape* rect2 = make_bar(vb, light_grey, convert_resolution(sf::Vector2f(1920 - 50, 75)));
    sf::RectangleShape* rect3 = make_bar(hb, light_grey, convert_resolution(sf::Vector2f(1920 - 50, 75 + 75)));
    sf::RectangleShape* rect4 = make_bar(backdrop, dark_grey, convert_resolution(sf::Vector2f(1920 - 50, 75)));

    // Bars surrounding the general UI
    sf::Vector2f bg =  convert_resolution(sf::Vector2f(650, 650));
    sf::Vector2f hbm = convert_resolution(sf::Vector2f(650, 7));
    sf::Vector2f sb =  convert_resolution(sf::Vector2f(7, 650));

    // General Box Environment
    sf::RectangleShape* background = make_bar(bg, dark_grey, convert_resolution(sf::Vector2f(1920, 0)));
    sf::RectangleShape* topbar =     make_bar(hbm, light_grey, convert_resolution(sf::Vector2f(1920, 0)));
    sf::RectangleShape* bottombar =  make_bar(hbm, light_grey, convert_resolution(sf::Vector2f(1920, 650)));
    sf::RectangleShape* sidebar =    make_bar(sb, light_grey, convert_resolution(sf::Vector2f(1920, 0)));

    // General Button Sizes
    sf::Vector2f plusminus = convert_resolution(sf::Vector2f(20, 40));
    sf::Vector2f colorbox =  convert_resolution(sf::Vector2f(40, 40));

    // RGB Buttons
    sf::RectangleShape* red =    make_bar(colorbox, pastel_red, convert_resolution(sf::Vector2f(1920 + 75, 125)));
    sf::RectangleShape* rplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 115, 125)));
    sf::RectangleShape* rminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 55, 125)));
    sf::RectangleShape* green =  make_bar(colorbox, pastel_green, convert_resolution(sf::Vector2f(1920 + 190, 125)));
    sf::RectangleShape* gplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 230, 125)));
    sf::RectangleShape* gminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 170, 125)));
    sf::RectangleShape* blue =   make_bar(colorbox, pastel_blue, convert_resolution(sf::Vector2f(1920 + 305, 125)));
    sf::RectangleShape* bplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 345, 125)));
    sf::RectangleShape* bminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 285, 125)));

    // Radius, Size, and Velocity
    sf::RectangleShape* radius =    make_bar(colorbox, pastel_orange, convert_resolution(sf::Vector2f(1920 + 132, 200)));
    sf::RectangleShape* radplus =   make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 172, 200)));
    sf::RectangleShape* radminus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 112, 200)));
    sf::RectangleShape* mass =      make_bar(colorbox, pastel_yellow, convert_resolution(sf::Vector2f(1920 + 247, 200)));
    sf::RectangleShape* massplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 287, 200)));
    sf::RectangleShape* massminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 227, 200)));
    sf::RectangleShape* velocityx = make_bar(colorbox, purple, convert_resolution(sf::Vector2f(1920 + 132, 260)));
    sf::RectangleShape* velxplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 172, 260)));
    sf::RectangleShape* velxminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 112, 260)));
    sf::RectangleShape* velocityy = make_bar(colorbox, purple, convert_resolution(sf::Vector2f(1920 + 247, 260)));
    sf::RectangleShape* velyplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 287, 260)));
    sf::RectangleShape* velyminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 227, 260)));

    // Number of Particles to Spawn in
    sf::RectangleShape* amount =   make_bar(colorbox, pastel_orange, convert_resolution(sf::Vector2f(1920 + 132, 320)));
    sf::RectangleShape* amtplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 172, 320)));
    sf::RectangleShape* amtminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 112, 320)));

    // Multiplier for variable adjustments
    sf::RectangleShape* multiplier = make_bar(colorbox, pastel_yellow, convert_resolution(sf::Vector2f(1920 + 247, 320)));
    sf::RectangleShape* multiplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 287, 320)));
    sf::RectangleShape* multiminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 227, 320)));

    // Gravity constant
    sf::RectangleShape* gravity =   make_bar(colorbox, pastel_yellow, convert_resolution(sf::Vector2f(1920 + 190, 380)));
    sf::RectangleShape* gravplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 230, 380)));
    sf::RectangleShape* gravminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 170, 380)));

    // Rainbow Mode
    sf::RectangleShape* rainbow = make_bar(colorbox, white, convert_resolution(sf::Vector2f(1920 + 190, 450)));

    // Temperature
    sf::RectangleShape* temperature = make_bar(colorbox, pastel_yellow, convert_resolution(sf::Vector2f(1920 + 190, 490)));
    sf::RectangleShape* tempplus =    make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 230, 490)));
    sf::RectangleShape* tempminus =   make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 170, 490)));

    // Wind Values
    sf::RectangleShape* windx =      make_bar(colorbox, purple, convert_resolution(sf::Vector2f(1920 + 132, 560)));
    sf::RectangleShape* windxplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 172, 560)));
    sf::RectangleShape* windxminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 112, 560)));
    sf::RectangleShape* windy =      make_bar(colorbox, purple, convert_resolution(sf::Vector2f(1920 + 247, 560)));
    sf::RectangleShape* windyplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 287, 560)));
    sf::RectangleShape* windyminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 227, 560)));

    // Left Bar

    // Colors
    //      light_grey
    //      dark_grey
    //      purple
    //      pastel_orange
    //      pastel_yellow
    //      pastel_red
    //      pastel_green
    //      pastel_blue
    //      white

    // General Box
    sf::Vector2f left_background = convert_resolution(sf::Vector2f(50, 50));
    sf::Vector2f left_foreground = convert_resolution(sf::Vector2f(44, 44));

    // Move Window Box
    sf::RectangleShape* mw_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 23)));
    sf::RectangleShape* mw_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 20)));

    // Draw Particles Box
    sf::RectangleShape* dp_fg =     make_bar(left_foreground, nice_green, convert_resolution(sf::Vector2f(3, 73)));
    sf::RectangleShape* dp_bg =     make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 70)));

    // Curves
    sf::RectangleShape* curve_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 123)));
    sf::RectangleShape* curve_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 120)));

    // Lines
    sf::RectangleShape* lines_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 173)));
    sf::RectangleShape* lines_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 170)));

    // Wind
    sf::RectangleShape* dw_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 223)));
    sf::RectangleShape* dw_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 220)));

    // Consumption
    sf::RectangleShape* consume_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 273)));
    sf::RectangleShape* consume_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 270)));

    // Explosion
    sf::RectangleShape* explode_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 323)));
    sf::RectangleShape* explode_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 320)));

    // -Mass
    sf::RectangleShape* negmass_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 373)));
    sf::RectangleShape* negmass_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 370)));

    // Radioactivity
    sf::RectangleShape* rad_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 423)));
    sf::RectangleShape* rad_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 420)));

    // Teleportating
    sf::RectangleShape* tele_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 473)));
    sf::RectangleShape* tele_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 470)));

    // Swapping
    sf::RectangleShape* swap_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 523)));
    sf::RectangleShape* swap_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 520)));

    // Iridescence
    sf::RectangleShape* irid_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 573)));
    sf::RectangleShape* irid_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 570)));

    // Fire
    sf::RectangleShape* fire_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 623)));
    sf::RectangleShape* fire_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 620)));

    // Clear
    sf::RectangleShape* clear_fg = make_bar(left_foreground, light_grey, convert_resolution(sf::Vector2f(3, 673)));
    sf::RectangleShape* clear_bg = make_bar(left_background, dark_grey, convert_resolution(sf::Vector2f(0, 670)));

    // Preview Particles
    preview_particles.push_back(color_preview);
    preview_particles.push_back(draw_particle_preview);
    preview_particles.push_back(consume_preview);
    preview_particles.push_back(explode_preview);
    preview_particles.push_back(negative_mass_preview);
    preview_particles.push_back(radioactive_preview);
    preview_particles.push_back(teleport_preview);
    preview_particles.push_back(swap1_preview);
    preview_particles.push_back(swap2_preview);
    preview_particles.push_back(iridescent_preview);
    preview_particles.push_back(fire_preview);

    // Right Tab
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
    vectors[10].push_back(gravminus);
    vectors[10].push_back(gravity);
    vectors[10].push_back(gravplus);
    vectors[11].push_back(rainbow);
    vectors[12].push_back(tempminus);
    vectors[12].push_back(temperature);
    vectors[12].push_back(tempplus);
    vectors[13].push_back(windxminus);
    vectors[13].push_back(windx);
    vectors[13].push_back(windxplus);
    vectors[13].push_back(windyminus);
    vectors[13].push_back(windy);
    vectors[13].push_back(windyplus);

    // Left Bar
    vectors[14].push_back(mw_bg);
    vectors[14].push_back(mw_fg);
    vectors[15].push_back(dp_bg);
    vectors[15].push_back(dp_fg);
    vectors[16].push_back(curve_bg);
    vectors[16].push_back(curve_fg);
    vectors[17].push_back(lines_bg);
    vectors[17].push_back(lines_fg);
    vectors[18].push_back(dw_bg);
    vectors[18].push_back(dw_fg);
    vectors[19].push_back(consume_bg);
    vectors[19].push_back(consume_fg);
    vectors[20].push_back(explode_bg);
    vectors[20].push_back(explode_fg);
    vectors[21].push_back(negmass_bg);
    vectors[21].push_back(negmass_fg);
    vectors[22].push_back(rad_bg);
    vectors[22].push_back(rad_fg);
    vectors[23].push_back(tele_bg);
    vectors[23].push_back(tele_fg);
    vectors[24].push_back(swap_bg);
    vectors[24].push_back(swap_fg);
    vectors[25].push_back(irid_bg);
    vectors[25].push_back(irid_fg);
    vectors[26].push_back(fire_bg);
    vectors[26].push_back(fire_fg);
    vectors[27].push_back(clear_bg);
    vectors[27].push_back(clear_fg);
}

void UserInterface::TextDisplay(int start_vel_x, int start_vel_y, int mass, int radius, int modifier, int particle_amount, int red, int green, int blue, float gravity, int temperature) {
    sf::Text* velxstring = new sf::Text(std::to_string(start_vel_x), font);
    sf::Text* velystring = new sf::Text(std::to_string(start_vel_y), font);
    sf::Text* masstring = new sf::Text(std::to_string(mass), font);
    sf::Text* radiustring = new sf::Text(std::to_string(radius), font);
    sf::Text* modistring = new sf::Text(std::to_string(modifier), font);
    sf::Text* part_amt_string = new sf::Text(std::to_string(particle_amount), font);
    sf::Text* red_string = new sf::Text(std::to_string(red), font);
    sf::Text* green_string = new sf::Text(std::to_string(green), font);
    sf::Text* blue_string = new sf::Text(std::to_string(blue), font);
    sf::Text* grav_string = new sf::Text(std::to_string(gravity), font);
    sf::Text* temp_string = new sf::Text(std::to_string(temperature), font);

    custom_message(velxstring, convert_resolution(sf::Vector2f(1920 + 132, 265)));
    custom_message(velystring, convert_resolution(sf::Vector2f(1920 + 247, 265)));
    custom_message(masstring, convert_resolution(sf::Vector2f(1920 + 247, 205)));
    custom_message(radiustring, convert_resolution(sf::Vector2f(1920 + 132, 205)));
    custom_message(modistring, convert_resolution(sf::Vector2f(1920 + 247, 325)));
    custom_message(part_amt_string, convert_resolution(sf::Vector2f(1920 + 132, 325)));
    custom_message(red_string, convert_resolution(sf::Vector2f(1920 + 75, 130)));
    custom_message(green_string, convert_resolution(sf::Vector2f(1920 + 190, 130)));
    custom_message(blue_string, convert_resolution(sf::Vector2f(1920 + 305, 130)));
    custom_message(grav_string, convert_resolution(sf::Vector2f(1920 + 190, 385)));
    custom_message(temp_string, convert_resolution(sf::Vector2f(1920 + 190, 465)));

    texts.push_back(red_string);
    texts.push_back(green_string);
    texts.push_back(blue_string);
    texts.push_back(radiustring);
    texts.push_back(masstring);
    texts.push_back(velxstring);
    texts.push_back(velystring);
    texts.push_back(part_amt_string);
    texts.push_back(modistring);
    texts.push_back(grav_string);
    texts.push_back(temp_string);
}

void UserInterface::create_UI(int start_vel_x, int start_vel_y, int mass, int radius, int modifier, int particle_amount, int red, int green, int blue, float gravity, int temperature) {
    PictureDisplay();
    TextDisplay(start_vel_x, start_vel_y, mass, radius, modifier, particle_amount, red, green, blue, gravity, temperature);
}