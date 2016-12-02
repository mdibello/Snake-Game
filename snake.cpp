// Matthew DiBello
// Snake Game

#include <iostream>
#include <vector>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;

const int GRID_WIDTH = 32;
const int GRID_HEIGHT = 32;
const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 900;
const int NUMBER_OF_SNAKES = 4;
const sf::Color COLOR_ONE(0, 25, 51);
const sf::Color COLOR_TWO(32, 32, 32);

enum Obj { EMPTY, FRUIT, SNAKE };
enum Direction { UP, DOWN, LEFT, RIGHT };

struct Coord {
	int x;
	int y;
};

class Grid;
class Object;
class Snake;
class Fruit;

void render(sf::RenderWindow& window, Grid world);
void drawGrid(sf::RenderWindow& window, Grid world);
int coordToIndex(Coord c);
Coord indexToPixel(int i);
Coord indexToCoord(int i);

class Grid {
	public:
		Grid() { createGrid(10, 10); }
		Grid(int w) { createGrid(w, w); }
		Grid(int w, int h) { createGrid(w, h); }
		void createGrid(int w, int h);
		Object* getObject(int index) { return grid[index]; }
		void setObject(int index, Object* object) { grid[index] = object; }
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
		Object(int index) : location(indexToCoord(index)) { cout << "Object created" << endl; }
		virtual void move() = 0;
		virtual void draw(sf::RenderWindow& window, int index) = 0;
		virtual Obj whatAmI() = 0;
		void setLocation(Coord c) { location = c; }
	private:
		Coord location;
};

class Empty : public Object {
	public:
		Empty(int index) : Object(index), colorOne(COLOR_ONE), colorTwo(COLOR_TWO) { cout << "Empty created" << endl;}
		void move() { return; }
		void draw(sf::RenderWindow& window, int index);
		Obj whatAmI() { return EMPTY; }
	private:
		sf::Color colorOne;
		sf::Color colorTwo;
};

class Snake : public Object {
	public:
		Snake(Coord c, int playerNumber);
		void move() { return; }
		void draw(sf::RenderWindow& window, int index);
		Obj whatAmI() { return SNAKE; }
	private:
		vector<Coord> body;
		int playerID;
		sf::Color color;
};

class Fruit : public Object {
	public:
		void move();
		void draw(int index);
		Obj whatAmI() { return FRUIT; }
	private:
};

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
	//populateGrid(world);

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
					cout << "Keypress" << endl;
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
	for (int i = 0; i < world.getSize(); i++) {
		world.getObject(i)->draw(window, i);
	}
	window.display();
}

void populateGrid(Grid& world) {
	for (int i = 0; i < world.getSize(); i++) {
		Empty* e = new Empty(i);
		world.setObject(i, e);
		world.addToModified(i);
	}
}

int coordToIndex(Coord c) { 
	return (((GRID_WIDTH - 1 - c.y) * GRID_WIDTH) + c.x); 
}

Coord indexToPixel(int i) { 
	Coord c = {(i % GRID_WIDTH) * (SCREEN_WIDTH / GRID_WIDTH), (i / GRID_WIDTH) * (SCREEN_HEIGHT / GRID_HEIGHT)};
	return c;
}

Coord indexToCoord(int i) {
	Coord c = {(i % GRID_WIDTH), (i / GRID_WIDTH)};
	return c;
}

void Grid::createGrid(int w, int h) {
	Grid::width = w;
	Grid::height = h;
	// initialize word
	for (int i = 0; i < (Grid::width * Grid::height); i++) {
		cout << i << endl;
		Object* o = new Empty(i);
		Grid::grid.push_back(o);
	}
	// add snakes
	Coord coord;
	for (int i = 0; i < NUMBER_OF_SNAKES; i++) {
		switch (i) {
			case 0:
				coord = { GRID_WIDTH / 4, 3 * (GRID_HEIGHT / 4) };
				break;
			case 1:
				coord = { 3 * (GRID_WIDTH / 4), GRID_HEIGHT / 4 };
				break;
			case 2:
				coord = { GRID_WIDTH / 4, GRID_HEIGHT / 4 };
				break;
			case 3:
				coord = { 3 * (GRID_WIDTH / 4), 3 * (GRID_HEIGHT / 4) };
				break;
			default:
				cout << "The desired number of snakes (" << NUMBER_OF_SNAKES << ") is not supported." << endl;
				cout << "Exiting..." << endl;
				exit(1);
		}
		Object* snake = new Snake(coord, i);
		Object* temp = getObject(coordToIndex(coord));
		setObject(coordToIndex(coord), snake);
	}
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

Snake::Snake(Coord c, int playerNumber) : Object(coordToIndex(c)) {
	Snake::playerID = playerNumber;
	Snake::body;
	switch (Snake::playerID) {
		case 0:
			Snake::color = sf::Color(154, 205, 50); // yellowgreen
			break;
		case 1:
			Snake::color = sf::Color(85, 107, 47); // darkolivegreen
			break;
		case 2:
			Snake::color = sf::Color(0, 100, 0); // darkgreen
			break;
		case 3:
			Snake::color = sf::Color(46, 139, 87); // seagreen
			break;
	}
	cout << "Snake created." << endl;
}

void Snake::draw(sf::RenderWindow& window, int index) {
	Coord c = indexToPixel(index);
	sf::RectangleShape rectangle(sf::Vector2f(SCREEN_WIDTH / GRID_WIDTH, SCREEN_HEIGHT / GRID_HEIGHT));
	rectangle.setPosition(c.x,c.y);
	rectangle.setFillColor(Snake::color);
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
				rectangle.setFillColor(COLOR_TWO);
			}
			else {
				rectangle.setFillColor(COLOR_ONE);
			}
			window.draw(rectangle);
		}
	}
}
