// Matthew DiBello
// Snake Game

#include <iostream>
#include <vector>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;

const int GRID_WIDTH = 20;
const int GRID_HEIGHT = 20;
const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 900;
const int NUMBER_OF_SNAKES = 1;
const sf::Color colorOne(0, 25, 51);
const sf::Color colorTwo(32, 32, 32);

enum Obj { EMPTY, FRUIT, SNAKE };
enum Direction { UP, DOWN, LEFT, RIGHT };

struct Coord {
	int x;
	int y;
};

class Object;

class Grid {
	public:
		Grid() : width(10), height(10) { createGrid(10, 10); }
		Grid(int w) : width(w), height(w) { createGrid(w, w); }
		Grid(int w, int h) : width(w), height(h) { createGrid(w, h); }
		void createGrid(int w, int h) { vector<Object*> grid(w * h); }
		Object* getObject(int i);
		void setObject(int index, Object* object);
		int getHeight() { return height; }
		int getWidth() { return width; }
		int getSize() { return grid.size(); }
		void addToModified(int index) { modified.push_back(1); }
		int getFromModified(int index) { return modified[index]; }
		int getNumModified() { return modified.size(); }
	private:
		vector<Object*> grid;
		vector<int> modified;
		int height;
		int width;
};

class Object {
	public:
		virtual void move() = 0;
		virtual void draw(sf::RenderWindow& window, int index) = 0;
		virtual Obj whatAmI() = 0;
	private:
		Coord location;
};

class Empty : public Object {
	public:
		void move() { return; }
		void draw(sf::RenderWindow& window, int index);
		Obj whatAmI() { return EMPTY; }
	private:
};

class Snake : public Object {
	public:
		void move();
		void draw(int index);
		Obj whatAmI() { return SNAKE; }
	private:
		vector<Coord> body;
};

class Fruit : public Object {
	public:
		void move();
		void draw(int index);
		Obj whatAmI() { return FRUIT; }
	private:
};

void render(sf::RenderWindow& window, Grid world);
void populateGrid(Grid& world);
void drawGrid(sf::RenderWindow& window, Grid world);
int coordToIndex(Coord c);
Coord indexToPixel(int i);

int main() {
	
	sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Matt DiBello's Snake Game", 
		sf::Style::Titlebar | sf::Style::Close);
	// sync application refresh rate with monitor
	window.setVerticalSyncEnabled(true);
	// limit refresh rate to 60 fps
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);

	window.clear(sf::Color::Green);
	window.display();
	
	Grid world(GRID_WIDTH, GRID_HEIGHT);
	populateGrid(world);

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {

			switch (event.type) {
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::Resized:
					cout << "Window resized to " << event.size.width << " X " << event.size.height << endl;
					break;
				case sf::Event::LostFocus:
					// pause?
					break;
				case sf::Event::GainedFocus:
					// resume?
					break;
				case sf::Event::KeyPressed:
					render(window, world);
					break;
				default:
					break;
			}

		}
	}

	return 0;

}

void render(sf::RenderWindow& window, Grid world) {
	//drawGrid(window, world);
	for (int i = 0; i < world.getNumModified(); i++) {
		world.getObject(world.getFromModified(i))->draw(window, i);
	}
	window.display();
}

void populateGrid(Grid& world) {
	for (int i = 0; i < world.getSize(); i++) {
		Empty* e = new Empty();
		world.setObject(i, e);
		world.addToModified(i);
	}
}

Coord indexToPixel(int i) { 
	Coord c = {(i % GRID_WIDTH) * (SCREEN_WIDTH / GRID_WIDTH), (i / GRID_WIDTH) * (SCREEN_HEIGHT / GRID_HEIGHT)};
	return c;
}

int coordToIndex(Coord c) { 
	return (((GRID_WIDTH - 1 - c.y) * GRID_WIDTH) + c.x); 
}

void Grid::setObject(int index, Object* o) {
	this->grid[index] = o;
}

Object* Grid::getObject(int index) {
	return this->grid[index];
}

void Empty::draw(sf::RenderWindow& window, int index) {
	sf::Color color = colorOne;
	Coord c = indexToPixel(index);
	if (GRID_WIDTH % 2 == 0) {
		if (((index / GRID_WIDTH) + (index % GRID_WIDTH)) % 2 == 0) {
			color = colorTwo;
		}
	}
	else {
		if (index % 2 == 0) {
			color = colorTwo;
		}
	}
	sf::RectangleShape rectangle(sf::Vector2f(SCREEN_WIDTH / GRID_WIDTH, SCREEN_HEIGHT / GRID_HEIGHT));
	rectangle.setPosition(c.x,c.y);
	rectangle.setFillColor(color);
	window.draw(rectangle);
	
}

void drawGrid(sf::RenderWindow& window, Grid world) {
	float tile_height = SCREEN_HEIGHT / GRID_HEIGHT;
	float tile_width = SCREEN_WIDTH / GRID_WIDTH;
	for (int i = 0; i < world.getHeight(); i++) {
		for (int j = 0; j < world.getWidth(); j++) {
			sf::RectangleShape rectangle(sf::Vector2f(tile_width, tile_height));
			rectangle.setPosition(tile_width * j, tile_height * i);
			if ((i + j) % 2 == 0) {
				rectangle.setFillColor(colorTwo);
			}
			else {
				rectangle.setFillColor(colorOne);
			}
			window.draw(rectangle);
		}
	}
}
