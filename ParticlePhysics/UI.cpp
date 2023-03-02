#include "UI.h"

UserInterface::UserInterface(sf::Vector2f ws) {
    windowsize = ws;
    if (!font.loadFromFile("C:\\WINDOWS\\FONTS\\ARIAL.TTF")) {}
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
sf::RectangleShape UserInterface::make_bar(sf::Vector2f size, sf::Color color, sf::Vector2f position) {
    sf::RectangleShape rectangle(size);
    rectangle.setFillColor(color);
    rectangle.setPosition(position);
    return rectangle;
}
void custom_message(sf::Text* message, sf::Vector2f position) {
    message->setCharacterSize(15);
    message->setFillColor(sf::Color::Black);
    message->setPosition(position);
}
void UserInterface::PictureDisplay() {

    particle_preview->setRadius(5);
    particle_preview->setFillColor(sf::Color(255, 0, 0));
    particle_preview->setPosition(convert_resolution(sf::Vector2f(1920 + 210 - 5, 70 - 5)));

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
    vectors.push_back(std::vector<sf::RectangleShape>()); // Boxes to Change Gravity        [10]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Box to toggle rainbow mode     [11]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Box to toggle temperature      [12]
    vectors.push_back(std::vector<sf::RectangleShape>()); // Box to toggle wind             [13]


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
    sf::Vector2f bg =  convert_resolution(sf::Vector2f(650, 650));
    sf::Vector2f hbm = convert_resolution(sf::Vector2f(650, 7));
    sf::Vector2f sb =  convert_resolution(sf::Vector2f(7, 650));

    // General Box Environment
    sf::RectangleShape background = make_bar(bg, dark_grey, convert_resolution(sf::Vector2f(1920, 0)));
    sf::RectangleShape topbar =     make_bar(hbm, light_grey, convert_resolution(sf::Vector2f(1920, 0)));
    sf::RectangleShape bottombar =  make_bar(hbm, light_grey, convert_resolution(sf::Vector2f(1920, 650)));
    sf::RectangleShape sidebar =    make_bar(sb, light_grey, convert_resolution(sf::Vector2f(1920, 0)));

    // General Button Sizes
    sf::Vector2f plusminus = convert_resolution(sf::Vector2f(20, 40));
    sf::Vector2f colorbox =  convert_resolution(sf::Vector2f(40, 40));

    // RGB Buttons
    sf::RectangleShape red =    make_bar(colorbox, pastel_red, convert_resolution(sf::Vector2f(1920 + 75, 125)));
    sf::RectangleShape rplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 115, 125)));
    sf::RectangleShape rminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 55, 125)));
    sf::RectangleShape green =  make_bar(colorbox, pastel_green, convert_resolution(sf::Vector2f(1920 + 190, 125)));
    sf::RectangleShape gplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 230, 125)));
    sf::RectangleShape gminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 170, 125)));
    sf::RectangleShape blue =   make_bar(colorbox, pastel_blue, convert_resolution(sf::Vector2f(1920 + 305, 125)));
    sf::RectangleShape bplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 345, 125)));
    sf::RectangleShape bminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 285, 125)));

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
    sf::RectangleShape amount =   make_bar(colorbox, pastel_orange, convert_resolution(sf::Vector2f(1920 + 132, 320)));
    sf::RectangleShape amtplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 172, 320)));
    sf::RectangleShape amtminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 112, 320)));

    // Multiplier for variable adjustments
    sf::RectangleShape multiplier = make_bar(colorbox, pastel_yellow, convert_resolution(sf::Vector2f(1920 + 247, 320)));
    sf::RectangleShape multiplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 287, 320)));
    sf::RectangleShape multiminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 227, 320)));

    // Gravity constant
    sf::RectangleShape gravity =   make_bar(colorbox, pastel_yellow, convert_resolution(sf::Vector2f(1920 + 190, 380)));
    sf::RectangleShape gravplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 230, 380)));
    sf::RectangleShape gravminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 170, 380)));

    // Rainbow Mode
    sf::RectangleShape rainbow = make_bar(colorbox, white, convert_resolution(sf::Vector2f(1920 + 190, 450)));

    // Temperature
    sf::RectangleShape temperature = make_bar(colorbox, pastel_yellow, convert_resolution(sf::Vector2f(1920 + 190, 490)));
    sf::RectangleShape tempplus =    make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 230, 490)));
    sf::RectangleShape tempminus =   make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 170, 490)));

    // Wind
    sf::RectangleShape windx =      make_bar(colorbox, purple, convert_resolution(sf::Vector2f(1920 + 132, 560)));
    sf::RectangleShape windxplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 172, 560)));
    sf::RectangleShape windxminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 112, 560)));
    sf::RectangleShape windy =      make_bar(colorbox, purple, convert_resolution(sf::Vector2f(1920 + 247, 560)));
    sf::RectangleShape windyplus =  make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 287, 560)));
    sf::RectangleShape windyminus = make_bar(plusminus, light_grey, convert_resolution(sf::Vector2f(1920 + 227, 560)));

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