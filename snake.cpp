// Matthew DiBello
// Snake Game

#include <iostream>
#include <vector>
#include <utility>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;

const int GRID_WIDTH = 32;
const int GRID_HEIGHT = 32;
const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 960;
const int NUMBER_OF_SNAKES = 1;
const sf::Color COLOR_ONE(0, 25, 51);
const sf::Color COLOR_TWO(32, 32, 32);

enum Obj { EMPTY, FRUIT, SNAKE };
enum Direction { UP, DOWN, LEFT, RIGHT };
enum Result { SUCCESS, PLAYER_ONE_LOSES, PLAYER_TWO_LOSES, PLAYER_THREE_LOSES, PLAYER_FOUR_LOSES, FAILURE };

struct Coord {
	int x;
	int y;
};

class Grid;
class Object;
class Snake;
class Fruit;

void render(sf::RenderWindow& window, Grid world);
void takeTurn(Grid world);
void timeThread(pair<sf::RenderWindow&, Grid> p);
int coordToIndex(Coord c);
Coord indexToPixel(int i);
Coord indexToCoord(int i);
bool locationIsInvalid(Coord c);

class Grid {
	public:
		Grid() { createGrid(10, 10); }
		Grid(int w) { createGrid(w, w); }
		Grid(int w, int h) { createGrid(w, h); }
		void createGrid(int w, int h);
		Object* getObject(int index) { return grid[index]; }
		void setObject(int index, Object* object) { grid[index] = object; }
		void swapObjects(int indexOne, int indexTwo);
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
		virtual Result move(Grid world) = 0;
		virtual void draw(sf::RenderWindow& window, int index) = 0;
		virtual Obj whatAmI() = 0;
		void setLocation(Coord c) { location = c; }
		Coord getLocation() { return location; }
	private:
		Coord location;
};

class Empty : public Object {
	public:
		Empty(int index) : Object(index), colorOne(COLOR_ONE), colorTwo(COLOR_TWO) { cout << "Empty created" << endl;}
		Result move(Grid world) { return FAILURE; }
		void draw(sf::RenderWindow& window, int index);
		Obj whatAmI() { return EMPTY; }
	private:
		sf::Color colorOne;
		sf::Color colorTwo;
};

class Snake : public Object {
	public:
		Snake(Coord c, int playerNumber);
		Result move(Grid world);
		void draw(sf::RenderWindow& window, int index);
		Obj whatAmI() { return SNAKE; }
	private:
		vector<Coord> body;
		int playerID;
		sf::Color color;
		Direction direction;
};

class Fruit : public Object {
	public:
		Result move(Grid world);
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

	window.clear(sf::Color::White);
	window.display();
	
	Grid world(GRID_WIDTH, GRID_HEIGHT);

	window.setActive(false);
	sf::Thread timingThread(&timeThread, pair<sf::RenderWindow&, Grid>( window, world ));
	timingThread.launch();

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
		if (world.getObject(i)->whatAmI() == SNAKE) {
			cout << "Snake: " << i << endl;
		}
	}
	cout << "Rendered world" << endl;
	window.display();
}

void timeThread(pair<sf::RenderWindow&, Grid> p) {
	// initial turn length is 1 second
	sf::RenderWindow& window = p.first;
	Grid world = p.second;
	
	sf::Time turnLength = sf::milliseconds(1000);
	sf::Clock clock;
	
	while(window.isOpen()) {
		if (clock.getElapsedTime() >= turnLength) {
			takeTurn(world);
			render(window, world);
			clock.restart();
		}
	}
}

void takeTurn(Grid world) {
	// move snakes
	cout << "Took turn" << endl;
	Object* obj;
	for (int i = 0; i < world.getSize(); i++) {
		obj = world.getObject(i);
		if (obj->whatAmI() == SNAKE) {
			cout << "Move Snake" << endl;
			obj->move(world);
		}
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

bool locationIsInvalid(Coord c) {
	if (c.x < 0 || c.x >= GRID_WIDTH || c.y < 0 || c.y >= GRID_HEIGHT) {
		return true;
	}
	return false;
}

void Grid::createGrid(int w, int h) {
	Grid::width = w;
	Grid::height = h;
	// initialize world
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

void Grid::swapObjects(int indexOne, int indexTwo) {
	Coord temp_location;
	Object* temp_object_ptr;
	// swap pointers
	temp_object_ptr = Grid::getObject(indexOne);
	Grid::setObject(indexOne, Grid::getObject(indexTwo));
	Grid::setObject(indexTwo, temp_object_ptr);
	// swap locations
	temp_location = Grid::getObject(indexOne)->getLocation();
	Grid::getObject(indexOne)->setLocation(Grid::getObject(indexTwo)->getLocation());
	Grid::getObject(indexTwo)->setLocation(temp_location);
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
			Snake::direction = DOWN;
			break;
		case 1:
			Snake::color = sf::Color(85, 107, 47); // darkolivegreen
			Snake::direction = UP;
			break;
		case 2:
			Snake::color = sf::Color(0, 100, 0); // darkgreen
			Snake::direction = RIGHT;
			break;
		case 3:
			Snake::color = sf::Color(46, 139, 87); // seagreen
			Snake::direction = LEFT;
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
	// draw body
}

Result Snake::move(Grid world) {
	Coord new_location = Snake::getLocation();
	cout << Snake::direction << endl;
	switch (Snake::direction) {
		case UP:
			new_location.y++;
			break;
		case DOWN:
			new_location.y--;
			break;
		case LEFT:
			new_location.x--;
			break;
		case RIGHT:
			new_location.x++;
			break;
	}
	if (locationIsInvalid(new_location)) {
		cout << "Location is invalid" << endl;
		return static_cast<Result>(Snake::playerID);
	}
	cout << coordToIndex(Snake::getLocation()) << " -> " << coordToIndex(new_location) << endl;
	world.swapObjects(coordToIndex(Snake::getLocation()), coordToIndex(new_location));
	Snake::setLocation(new_location);
	// move body loop
	return SUCCESS;
}
