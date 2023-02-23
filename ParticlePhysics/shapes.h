#pragma once
#ifndef __SHAPES_H__
#define __SHAPES_H__

class Shape {
public:

    sf::VertexArray* shape;
    int segments;
    bool p0_set = false;
    bool p1_set = false;
    bool p2_set = false;

    // Determines if the shape is ready to draw
    bool drawable = false;

    Shape() = default;
    Shape(sf::VertexArray* aShape, int aSegments) {
        shape = aShape;
        segments = aSegments;
    }
};

class Bezier_Curve : public Shape {
public:

    sf::Vector2f p0; // starting point
    sf::Vector2f p1; // intermediate point
    sf::Vector2f p2; // ending point

    Bezier_Curve(sf::VertexArray* aShape, int aSegments) {
        shape = aShape;
        segments = aSegments;
    }
    void add_point(sf::Vector2i point) {
        
        if (!p0_set) {
            p0_set = true;
            p0 = sf::Vector2f(point.x, point.y);
        }
        else if (!p1_set) {
            p1_set = true;
            p1 = sf::Vector2f(point.x, point.y);
        }
        else if (!p2_set) {
            p2_set = true;
            p2 = sf::Vector2f(point.x, point.y);
            drawable = true;
            create_curve();
        }
    }

    void create_curve() {
        (*shape)[0].position = p0;
        for (int i = 1; i < segments; i++) {
            float t = static_cast<float>(i) / 49.0f; // calculate the t parameter for the curve
            sf::Vector2f point = std::pow(1.0f - t, 2.0f) * p0 + 2.0f * (1.0f - t) * t * p1 + std::pow(t, 2.0f) * p2; // calculate the point on the curve
            (*shape)[i].position = point; // add the point to the vertex array
        }
    }
};

class Line : public Shape {
public:

    sf::Vector2f p0; // starting point
    sf::Vector2f p1; // ending point

    Line(sf::VertexArray* aShape, int aSegments) {
        shape = aShape;
        segments = aSegments;
    }
    void add_point(sf::Vector2i point) {
        if (!p0_set) {
            p0_set = true;
            p0 = sf::Vector2f(point.x, point.y);
        }
        else if (!p1_set) {
            p1_set = true;
            p1 = sf::Vector2f(point.x, point.y);
            drawable = true;
            create_line();
        }
    }

    void create_line() {
        (*shape)[0].position = p0;
        float x_dif = (p1.x - p0.x) / (segments - 1);
        float y_dif = (p1.y - p0.y) / (segments - 1);
        float cur_x = p0.x;
        float cur_y = p0.y;
        for (int i = 1; i < segments; i++) {
            cur_x += x_dif;
            cur_y += y_dif;
            sf::Vector2f point(cur_x, cur_y);
            (*shape)[i].position = point;
        }
    }
};

#endif // !__SHAPES_H__
