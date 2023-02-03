#ifndef __QTREE_H__
#define __QTREE_H__

#include <SFML/Graphics.hpp>
#include <vector>
#include <numeric>
#include <algorithm>
#include <exception>
#include <functional>

// BB = Bounding Box

class Point {
public:
	float x, y;
	Particle* particle;

	Point(sf::Vector2f pos, Particle* p) {
		x = pos.x;
		y = pos.y;
		particle = p;
	}
};

class CircleBB {
public:
	// Constructor
	float x, y, r, rsquared;

	CircleBB() = default;
	CircleBB(sf::Vector2f pos, float radius) {
		x = pos.x;
		y = pos.y;
		r = radius;
		rsquared = radius * radius;
	}

	bool intersects(CircleBB range) {

	}

	bool contains(Point point) {
		float d = pow((point.x - x), 2) + pow((point.y - y), 2);
		return d <= rsquared;
	}
};

class RectangleBB {
public:
	float x, y, w, h;
	float left, right, top, bottom;
	// Constructor
	RectangleBB(sf::Vector2f pos, int width, int height) {
		x = pos.x;
		y = pos.y;
		w = width;
		h = height;

		left = x - w / 2;
		right = x + w / 2;
		top = y - h / 2;
		bottom = y + h / 2;
	}

	bool contains(Point point) {
		return (point.x > x - w &&
			point.x < x + w &&
			point.y > y - h &&
			point.y < y + h);
	}

	bool intersects(RectangleBB range) {
		return !(left <= range.left || range.right < left ||
			bottom < range.top || range.bottom < top);
	}
};

class QuadTree {
public:
	RectangleBB* boundary;
	int capacity;
	std::vector<Point> points;
	bool subdivided = false;

	// Children containing more particles
	QuadTree* northWest;
	QuadTree* northEast;
	QuadTree* southWest;
	QuadTree* southEast;

	// Constructor
	QuadTree(RectangleBB* bounds, int cap) {
		boundary = bounds;
		capacity = cap;
	}

	bool insert(Point point) {

		// Points don't belong in the tree
		if (!boundary->contains(point)) return false;

		// Hasn't reached capacity yet
		if (points.size() < capacity) {
			points.push_back(point);
			return true;
		}

		if (!subdivided) subdivide();

		if (northWest->insert(point)) return true;
		if (northEast->insert(point)) return true;
		if (southWest->insert(point)) return true;
		if (southEast->insert(point)) return true;
	}

	void subdivide() {
		subdivided = true;
		float x = boundary->x;
		float y = boundary->y;
		float w = boundary->w;
		float h = boundary->h;

		RectangleBB* NW = new RectangleBB(sf::Vector2f(x - w / 2, y - h / 2), w / 2, h / 2);
		RectangleBB* NE = new RectangleBB(sf::Vector2f(x + w / 2, y - h / 2), w / 2, h / 2);
		RectangleBB* SW = new RectangleBB(sf::Vector2f(x - w / 2, y + h / 2), w / 2, h / 2);
		RectangleBB* SE = new RectangleBB(sf::Vector2f(x + w / 2, y + h / 2), w / 2, h / 2);
		northWest = new QuadTree(NW, 4);
		northEast = new QuadTree(NE, 4);
		southWest = new QuadTree(SW, 4);
		southEast = new QuadTree(SE, 4);
	}

	std::vector<Point> queryRange(CircleBB range) {
		std::vector<Point> pointsInRange;

		// if (!boundary->intersects(range)) return pointsInRange;

		for (int p = 0; p < points.size(); p++) {
			if (range.contains(points[p])) pointsInRange.push_back(points[p]);
		}

		if (!subdivided) return pointsInRange;

		std::vector<Point> NWP = northWest->queryRange(range);
		std::vector<Point> NEP = northWest->queryRange(range);
		std::vector<Point> SWP = northWest->queryRange(range);
		std::vector<Point> SEP = northWest->queryRange(range);
		
		pointsInRange.reserve(NWP.size() + NEP.size() + SWP.size() + SEP.size());
		pointsInRange.insert(pointsInRange.end(), NWP.begin(), NWP.end());
		pointsInRange.insert(pointsInRange.end(), NEP.begin(), NEP.end());
		pointsInRange.insert(pointsInRange.end(), SWP.begin(), SWP.end());
		pointsInRange.insert(pointsInRange.end(), SEP.begin(), SEP.end());

		return pointsInRange;
	}
};

#endif // !__QTREE_H__