#ifndef __QTREE_H__
#define __QTREE_H__

// BB = Bounding Box

class Point {
public:
	float x, y;
	int index;

	Point(sf::Vector2f pos, int i) {
		x = pos.x;
		y = pos.y;
		index = i;
	}
};

//class CircleBB {
//public:
//	// Constructor
//	float x, y, r, rsquared;
//
//	CircleBB() = default;
//	CircleBB(sf::Vector2f pos, float radius) {
//		x = pos.x;
//		y = pos.y;
//		r = radius;
//		rsquared = radius * radius;
//	}
//
//	bool contains(Point* point) {
//		float d = pow((point->x - x), 2) + pow((point->y - y), 2);
//		return d <= rsquared;
//	}
//};

class RectangleBB {
public:
	float x, y, w, h;
	float left, right, top, bottom;

	// Constructor
	RectangleBB() = default;
	RectangleBB(sf::Vector2f pos, int width, int height) {
		x = pos.x;
		y = pos.y;
		w = width;
		h = height;
	}

	bool contains(Point point) {
		return (point.x >= x - w &&
			point.x <= x + w &&
			point.y >= y - h &&
			point.y <= y + h);
	}

	bool intersects(RectangleBB range) {
		return !(range.x - range.w > x + w ||
				 range.x + range.w < x - w ||
				 range.y - range.h > y + h ||
				 range.y + range.h < y - h);
	}
};

class QuadTree {
public:
	RectangleBB boundary;
	int capacity;
	std::vector<Point>* points = new std::vector<Point>;
	bool subdivided = false;

	// Children containing more particles
	QuadTree* northWest;
	QuadTree* northEast;
	QuadTree* southWest;
	QuadTree* southEast;

	// Constructor
	QuadTree() = default;
	QuadTree(RectangleBB bounds, int cap) {
		boundary = bounds;
		capacity = cap;
	}

	~QuadTree() {
		delete points;
		delete northWest;
		delete northEast;
		delete southWest;
		delete southEast;
	}

	bool insert(Point point) {

		// Points don't belong in the tree
		if (!boundary.contains(point)) return false;

		// Hasn't reached capacity yet
		if (points->size() < capacity && !subdivided) {
			points->push_back(point);
			return true;
		}

		if (!subdivided) subdivide();

		if (northWest->insert(point)) return true;
		if (northEast->insert(point)) return true;
		if (southWest->insert(point)) return true;
		if (southEast->insert(point)) return true;
	}

	void subdivide() {
		float x = boundary.x;
		float y = boundary.y;
		float w = boundary.w;
		float h = boundary.h;

		RectangleBB NE(sf::Vector2f(x + w / 2, y - h / 2), w / 2, h / 2);
		RectangleBB NW(sf::Vector2f(x - w / 2, y - h / 2), w / 2, h / 2);
		RectangleBB SE(sf::Vector2f(x + w / 2, y + h / 2), w / 2, h / 2);
		RectangleBB SW(sf::Vector2f(x - w / 2, y + h / 2), w / 2, h / 2);
		northWest = new QuadTree(NW, capacity);
		northEast = new QuadTree(NE, capacity);
		southWest = new QuadTree(SW, capacity);
		southEast = new QuadTree(SE, capacity);
		subdivided = true;
	}

	// Grab particles around another particle
	std::vector<Point>* queryRange(RectangleBB range, std::vector<Point>* pointsInRange) {

		if (!boundary.intersects(range)) return pointsInRange;

		for (int p = 0; p < points->size(); p++) {
			if (range.contains(points->at(p))) pointsInRange->push_back(points->at(p));
		}

		if (!subdivided) return pointsInRange;

		northWest->queryRange(range, pointsInRange);
		northEast->queryRange(range, pointsInRange);
		southWest->queryRange(range, pointsInRange);
		southEast->queryRange(range, pointsInRange);

		return pointsInRange;
	}
};

#endif // !__QTREE_H__