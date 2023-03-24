#ifndef __QTREE_H__
#define __QTREE_H__

// BB = Bounding Box

class Point {
public:
	float x, y;
	int index;
	float mass;

	Point(sf::Vector2f pos, int i, float a_mass) {
		x = pos.x;
		y = pos.y;
		index = i;
		mass = a_mass;
	}
};

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

class Barnes_Hut : public QuadTree {

protected:
	const float GRAV_CONST = 25.f;
	const float SOFTENER = .1f;
	const float THETA = 0.01f;

	// Children containing more particles
	Barnes_Hut* northWestBarnes;
	Barnes_Hut* northEastBarnes;
	Barnes_Hut* southWestBarnes;
	Barnes_Hut* southEastBarnes;

public:
	float x_sum = 0;
	float y_sum = 0;
	float total_mass = 0;
	sf::Vector2f center_of_mass = sf::Vector2f(0, 0);

	Barnes_Hut() = default;
	Barnes_Hut(RectangleBB bounds, int cap) {
		boundary = bounds;
		capacity = cap;
	}
	~Barnes_Hut() {
		delete northEastBarnes;
		delete northWestBarnes;
		delete southEastBarnes;
		delete southWestBarnes;
	}
	void update_com(Point point) {
		x_sum += point.x * point.mass;
		y_sum += point.y * point.mass;
		total_mass += abs(point.mass);
		center_of_mass.x = x_sum / total_mass;
		center_of_mass.y = y_sum / total_mass;
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
		northWestBarnes = new Barnes_Hut(NW, capacity);
		northEastBarnes = new Barnes_Hut(NE, capacity);
		southWestBarnes = new Barnes_Hut(SW, capacity);
		southEastBarnes = new Barnes_Hut(SE, capacity);
		subdivided = true;
	}
	void insert_recur(Point point) {
		if (northWestBarnes->insert(point)) return;
		if (northEastBarnes->insert(point)) return;
		if (southWestBarnes->insert(point)) return;
		if (southEastBarnes->insert(point)) return;
	}
	bool insert(Point point) {
		
		// Check if particle is within the boundary.
		if (!boundary.contains(point)) return false;

		// If particle is within the boundary, update the center of mass
		update_com(point);

		// No points in grid
		if (points->size() == 0 && !subdivided) {
			points->push_back(point);
			return true;
		}

		// Subdivide grid
		if (!subdivided) subdivide();

		// Moving Current Node then clearing the array
		for (int i = 0; i < points->size(); i++) {
			insert_recur((*points)[i]);
		}
		points->clear();

		// Placing New Node
		insert_recur(point);
		
		return true;
	}
	void calculate_force(int index, Particle* main_particle, float& x_shift, float& y_shift) {

		sf::Vector2f main_pos = main_particle->particle->getPosition();

		// Boundary contains particle that's not itself
		if (points->size() > 0 && (*points)[0].index != index) {
			
			Point temp_particle = (*points)[0];
			float temp_mass = temp_particle.mass;

			auto distance = std::sqrt((temp_particle.x - main_pos.x) * (temp_particle.x - main_pos.x) + (temp_particle.y - main_pos.y) * (temp_particle.y - main_pos.y));
			if (distance > .0001) {
				auto force = GRAV_CONST * main_particle->mass * temp_mass / ((distance * distance) + SOFTENER);

				x_shift += force * (temp_particle.x - main_pos.x) / abs(distance);
				y_shift += force * (temp_particle.y - main_pos.y) / abs(distance);
				return;
			}
		}
		else {
			float s = boundary.w;
			float d = std::sqrt((center_of_mass.x - main_pos.x) * (center_of_mass.x - main_pos.x) + (center_of_mass.y - main_pos.y) * (center_of_mass.y - main_pos.y));;
			if (d > .0001) {
				if (s / d <  THETA) {
					auto force = GRAV_CONST * main_particle->mass * total_mass / ((d * d) + SOFTENER);

					x_shift += force * (center_of_mass.x - main_pos.x) / abs(d);
					y_shift += force * (center_of_mass.y - main_pos.y) / abs(d);
				}
			}

			if (subdivided) {
				northEastBarnes->calculate_force(index, main_particle, x_shift, y_shift);
				northWestBarnes->calculate_force(index, main_particle, x_shift, y_shift);
				southEastBarnes->calculate_force(index, main_particle, x_shift, y_shift);
				southWestBarnes->calculate_force(index, main_particle, x_shift, y_shift);
			}
			return;
		}
	}
};

#endif // !__QTREE_H__