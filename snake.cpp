// Matthew DiBello
// Snake Game

#include <iostream>
#include <vector>
#include <utility>
#include <random>
#include <cmath>
#include <unistd.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;

const int GRID_WIDTH = 34;
const int GRID_HEIGHT = 34;
const int SCREEN_WIDTH = 680;
const int SCREEN_HEIGHT = 680;
const int NUMBER_OF_SNAKES = 1;
const int MAX_FRUIT = 6;
const float INITIAL_SPEED = 0.1;
const float SPEED_INCREASE_RATE = 0.98;
const sf::Color COLOR_ONE(0, 25, 51);
const sf::Color COLOR_TWO(32, 32, 32);

enum Obj { EMPTY, FRUIT, SNAKE, SNAKE_BODY };
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

void displayWelcomeScreen(sf::RenderWindow& window);
bool displayEndingScreen(sf::RenderWindow& window, Grid& world, vector<int> scores);
void render(sf::RenderWindow& window, Grid& world);
vector<Result> takeTurn(Grid& world);
void listenThread(pair<sf::RenderWindow&, Grid&> p);
int coordToIndex(Coord c);
Coord indexToPixel(int i);
Coord indexToCoord(int i);
bool locationIsInvalid(Coord c);

class Object {
	public:
		Object(int index) : location(indexToCoord(index)) {}
		virtual Result move(Grid& world) = 0;
		virtual void draw(sf::RenderWindow& window, int index) = 0;
		virtual Obj whatAmI() = 0;
		void setLocation(Coord c) { location = c; }
		Coord getLocation() { return location; }
		virtual void setDirection(Direction d) {}
		virtual Direction getDirection() {}
		virtual int getScore() {}
	private:
		Coord location;
};

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
		void processKeypress();
		void resetHasMoved();
		int getNumberSnakes() { return snakes.size(); }
		Object* getSnakeAt(int index) { snakes[index]; }
		void setSnakeDirection(int snakeNumber, Direction dir) { snakes[snakeNumber]->setDirection(dir); }
		Direction getSnakeDirection(int snakeNumber) { return snakes[snakeNumber]->getDirection(); }
		void clearFruitsVector() { fruits.clear(); }
		void addFruitToVector(Object* fruit) { fruits.push_back(fruit); }
		void addSnakeBodies();
		void replenishFruit();
		void addFruit();
		float getSpeed() { return speed; }
		void setSpeed(float s) { speed = s; }
		void setGameStatuses(vector<Result> gS) { gameStatuses = gS; }
		int getNumberGameStatuses() { return gameStatuses.size(); }
		int getSnakeScore(int i) { return snakes[i]->getScore(); };
	private:
		vector<Object*> grid;
		vector<int> modified;
		vector<Object*> snakes;
		vector<Object*> fruits;
		vector<Result> gameStatuses;
		int height;
		int width;
		float speed;
};

class Empty : public Object {
	public:
		Empty(int index) : Object(index), colorOne(COLOR_ONE), colorTwo(COLOR_TWO) {}
		Result move(Grid& world) { return FAILURE; }
		void draw(sf::RenderWindow& window, int index);
		Obj whatAmI() { return EMPTY; }
	private:
		sf::Color colorOne;
		sf::Color colorTwo;
};

class Snake : public Object {
	public:
		Snake(Coord c, int playerNumber);
		Result move(Grid& world);
		void draw(sf::RenderWindow& window, int index);
		Obj whatAmI() { return SNAKE; }
		void setDirection(Direction d) { Snake::newDirection = d; }
		Direction getDirection() { return direction; }
		void resetMoveStatus() { hasMovedYet = false; }
		sf::Color getColor() { return color; }
		vector<Coord> getBodyVector() { return body; }
		int getScore() { return fruitsEaten; }
	private:
		vector<Coord> body;
		int playerID;
		sf::Color color;
		Direction direction;
		Direction newDirection;
		bool hasMovedYet;
		int fruitsEaten;
};

class SnakeBody : public Object {
	public:
		SnakeBody(int index, sf::Color c) : Object(index) { color = c; }
		Result move(Grid& world) { return FAILURE; }
		void draw(sf::RenderWindow& window, int index) {}
		Obj whatAmI() { return SNAKE_BODY; }
	private:
		sf::Color color;
};

class Fruit : public Object {
	public:
		Fruit(Coord c) : Object(coordToIndex(c)), color(sf::Color::Red) {};
		Result move(Grid& world) {}
		void draw(sf::RenderWindow& window, int index);
		Obj whatAmI() { return FRUIT; }
	private:
		sf::Color color;
};

int main() {
	
	sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Matt DiBello's Snake Game", 
		sf::Style::Titlebar | sf::Style::Close);
	// sync application refresh rate with monitor
	window.setVerticalSyncEnabled(true);
	// limit refresh rate to 60 fps
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);

	srand(time(0));

	displayWelcomeScreen(window);
	Grid world(GRID_WIDTH, GRID_HEIGHT);
	vector<int> scores;
	bool userWishesToContinue = true;
	
	do {	
	
		sf::Event event;
		do {
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
					window.close();
					exit(1);
				}
			}
		} while (!sf::Keyboard::isKeyPressed(sf::Keyboard::Space));
	
		Grid world(GRID_WIDTH, GRID_HEIGHT);
	
		window.setActive(false);
		sf::Thread listeningThread(&listenThread, pair<sf::RenderWindow&, Grid&>( window, world ));
		listeningThread.launch();
	
		sf::Time turnLength = sf::seconds(world.getSpeed());
		sf::Clock clock;
		
		while(window.isOpen() && world.getNumberGameStatuses() == 0) {
			if (clock.getElapsedTime() >= turnLength) {
				turnLength = sf::seconds(world.getSpeed());
				world.setGameStatuses(takeTurn(world));
				render(window, world);
				clock.restart();
			}
		}
		cout << "Help1?" << endl;
		listeningThread.terminate();
		cout << "Help2?" << endl;

		scores.clear();
		for (int i = 0; i < world.getNumberSnakes(); i++) {
			scores.push_back(world.getSnakeScore(i));
		}
		cout << "Help3?" << endl;
		
		userWishesToContinue = displayEndingScreen(window, world, scores);
		cout << "Help4?" << endl;

	} while (userWishesToContinue);

	return 0;

}

void displayWelcomeScreen(sf::RenderWindow& window) {
	window.clear(sf::Color::White);
	sf::Font font;
	if (!font.loadFromFile("SugarpunchDEMO.otf")) {
	    cout << "Error loading font from file" << endl;
	}
	sf::Text welcome;
	welcome.setFont(font);
	welcome.setString("Matt DiBello's Snake Game");
	welcome.setCharacterSize(36);
	welcome.setFillColor(sf::Color::Red);
	welcome.setStyle(sf::Text::Bold);
	sf::FloatRect textRect = welcome.getLocalBounds();
	welcome.setOrigin(textRect.left + textRect.width/2.0, SCREEN_HEIGHT/3.0);
	welcome.setPosition(sf::Vector2f(SCREEN_WIDTH/2.0, SCREEN_HEIGHT/2.0));
	window.draw(welcome);
	sf::Text instructions;
	instructions.setFont(font);
	instructions.setString("Eat the red fruit, avoid collisions");
	instructions.setCharacterSize(24);
	instructions.setFillColor(sf::Color::Red);
	instructions.setStyle(sf::Text::Bold);
	textRect = instructions.getLocalBounds();
	instructions.setOrigin(textRect.left + textRect.width/2.0, SCREEN_HEIGHT/4.0);
	instructions.setPosition(sf::Vector2f(SCREEN_WIDTH/2.0, SCREEN_HEIGHT/2.0));
	window.draw(instructions);
	instructions.setString("Player 1\n---\nUP: W\nDOWN: S\nLEFT: A\nRIGHT: D");
	instructions.setCharacterSize(24);
	instructions.setFillColor(sf::Color::Red);
	instructions.setStyle(sf::Text::Bold);
	textRect = instructions.getLocalBounds();
	instructions.setOrigin(textRect.left + textRect.width/2.0, SCREEN_HEIGHT/7.0);
	instructions.setPosition(sf::Vector2f(SCREEN_WIDTH/3.5, SCREEN_HEIGHT/2.0));
	window.draw(instructions);
	instructions.setString("Player 2\n---\nUP: UP ARROW\nDOWN: DOWN ARROW\nLEFT: LEFT ARROW\nRIGHT: RIGHT ARROW");
	instructions.setCharacterSize(24);
	instructions.setFillColor(sf::Color::Red);
	instructions.setStyle(sf::Text::Bold);
	textRect = instructions.getLocalBounds();
	instructions.setOrigin(textRect.left + textRect.width/2.0, SCREEN_HEIGHT/7.0);
	instructions.setPosition(sf::Vector2f(SCREEN_WIDTH/1.5, SCREEN_HEIGHT/2.0));
	window.draw(instructions);
	instructions.setString("Press SPACE to play");
	instructions.setCharacterSize(24);
	instructions.setFillColor(sf::Color::Red);
	instructions.setStyle(sf::Text::Bold);
	textRect = instructions.getLocalBounds();
	instructions.setOrigin(textRect.left + textRect.width/2.0, SCREEN_HEIGHT/2.0);
	instructions.setPosition(sf::Vector2f(SCREEN_WIDTH/2.0, SCREEN_HEIGHT/0.75));
	window.draw(instructions);
	window.display();
}

bool displayEndingScreen(sf::RenderWindow& window, Grid& world, vector<int> scores) {
	//usleep(2000000);
	cout << "Help5?" << endl;
	window.clear(sf::Color::White);
	cout << "Help6?" << endl;
	sf::Font font;
	if (!font.loadFromFile("SugarpunchDEMO.otf")) {
	    cout << "Error loading font from file" << endl;
	}
	cout << "Help7?" << endl;
	sf::Text gameOver;
	gameOver.setFont(font);
	gameOver.setString("Game Over");
	gameOver.setCharacterSize(36);
	gameOver.setFillColor(sf::Color::Red);
	gameOver.setStyle(sf::Text::Bold);
	sf::FloatRect textRect = gameOver.getLocalBounds();
	gameOver.setOrigin(textRect.left + textRect.width/2.0, SCREEN_HEIGHT/3.0);
	gameOver.setPosition(sf::Vector2f(SCREEN_WIDTH/2.0, SCREEN_HEIGHT/2.0));
	window.draw(gameOver);
	cout << "Help8?" << endl;
	sf::Text instructions;
	instructions.setFont(font);
	instructions.setString("Player 1\n---\nFruit eaten: " + to_string(scores[0]));
	instructions.setCharacterSize(24);
	instructions.setFillColor(sf::Color::Red);
	instructions.setStyle(sf::Text::Bold);
	textRect = instructions.getLocalBounds();
	instructions.setOrigin(textRect.left + textRect.width/2.0, SCREEN_HEIGHT/7.0);
	instructions.setPosition(sf::Vector2f(SCREEN_WIDTH/3.5, SCREEN_HEIGHT/2.0));
	window.draw(instructions);
	cout << "Help9?" << endl;
	if (world.getNumberSnakes() > 1) {
		instructions.setString("Player 2\n---\nFruit eaten: " + to_string(scores[1]));
		instructions.setCharacterSize(24);
		instructions.setFillColor(sf::Color::Red);
		instructions.setStyle(sf::Text::Bold);
		textRect = instructions.getLocalBounds();
		instructions.setOrigin(textRect.left + textRect.width/2.0, SCREEN_HEIGHT/7.0);
		instructions.setPosition(sf::Vector2f(SCREEN_WIDTH/1.5, SCREEN_HEIGHT/2.0));
		window.draw(instructions);
	}
	cout << "Help10?" << endl;
	instructions.setString("Press SPACE to play again or q to quit");
	instructions.setCharacterSize(24);
	instructions.setFillColor(sf::Color::Red);
	instructions.setStyle(sf::Text::Bold);
	textRect = instructions.getLocalBounds();
	instructions.setOrigin(textRect.left + textRect.width/2.0, SCREEN_HEIGHT/2.0);
	instructions.setPosition(sf::Vector2f(SCREEN_WIDTH/2.0, SCREEN_HEIGHT/0.75));
	window.draw(instructions);
	cout << "Help11?" << endl;
	window.display();
	cout << "Help12?" << endl;
}

void render(sf::RenderWindow& window, Grid& world) {
	Object* obj;
	for (int i = 0; i < world.getSize(); i++) {
		obj = world.getObject(i);
		obj->draw(window, i);
	}
	window.display();
}

void listenThread(pair<sf::RenderWindow&, Grid&> p) {
	sf::RenderWindow& window = p.first;
	Grid world = p.second;	
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
				case sf::Event::Closed:
					window.close();
					break;
				case sf::Event::Resized:
					break;
				case sf::Event::LostFocus:
					// pause?
					break;
				case sf::Event::GainedFocus:
					// resume?
					break;
				case sf::Event::KeyPressed:
					switch (event.key.code) {
						case (sf::Keyboard::W) :
							if (world.getNumberSnakes() >= 1 && world.getSnakeDirection(0) != DOWN
								&& world.getSnakeDirection(0) != UP) {
								world.setSnakeDirection(0, UP);
							}
							break;
						case (sf::Keyboard::A) :
							if (world.getNumberSnakes() >= 1 && world.getSnakeDirection(0) != RIGHT
								&& world.getSnakeDirection(0) != LEFT) {
								world.setSnakeDirection(0, LEFT);
							}
							break;
						case (sf::Keyboard::S) :
							if (world.getNumberSnakes() >= 1 && world.getSnakeDirection(0) != UP
								&& world.getSnakeDirection(0) != DOWN) {
								world.setSnakeDirection(0, DOWN);
							}
							break;
						case (sf::Keyboard::D) :
							if (world.getNumberSnakes() >= 1 && world.getSnakeDirection(0) != LEFT
								&& world.getSnakeDirection(0) != RIGHT) {
								world.setSnakeDirection(0, RIGHT);
							}
							break;
						case (sf::Keyboard::Up) :
							if (world.getNumberSnakes() >= 2 && world.getSnakeDirection(1) != DOWN
								&& world.getSnakeDirection(1) != UP) {
								world.setSnakeDirection(1, UP);
							}
							break;
						case (sf::Keyboard::Left) :
							if (world.getNumberSnakes() >= 2 && world.getSnakeDirection(1) != RIGHT
								&& world.getSnakeDirection(1) != LEFT) {
								world.setSnakeDirection(1, LEFT);
							}
							break;
						case (sf::Keyboard::Down) :
							if (world.getNumberSnakes() >= 2 && world.getSnakeDirection(1) != UP
								&& world.getSnakeDirection(1) != DOWN) {
								world.setSnakeDirection(1, DOWN);
							}
							break;
						case (sf::Keyboard::Right) :
							if (world.getNumberSnakes() >= 2 && world.getSnakeDirection(1) != LEFT
								&& world.getSnakeDirection(1) != RIGHT) {
								world.setSnakeDirection(1, RIGHT);
							}
							break;
					}
					break;
				default:
					break;
			}
		}
	}
}

vector<Result> takeTurn(Grid& world) {
	Object* obj;
	world.clearFruitsVector();
	Obj objectType;
	Result turnStatus;
	vector<Result> statuses;
	for (int i = 0; i < world.getSize(); i++) {
		obj = world.getObject(i);
		objectType = obj->whatAmI();
		switch (objectType) {
			case SNAKE :
				turnStatus = obj->move(world);
				if (turnStatus != SUCCESS && turnStatus != FAILURE) {
					statuses.push_back(turnStatus);
				}
				break;
			case FRUIT :
				world.addFruitToVector(obj);
				break;
			case SNAKE_BODY :
				Object* empty = new Empty(coordToIndex(obj->getLocation()));
				Object* temp = world.getObject(coordToIndex(obj->getLocation()));
				delete temp;
				world.setObject(coordToIndex(obj->getLocation()), empty);
				break;
		}
	}
	world.addSnakeBodies();
	world.replenishFruit();
	world.resetHasMoved();
	return statuses;
}

int coordToIndex(Coord c) { 
	return (((GRID_WIDTH - 1 - c.y) * GRID_WIDTH) + c.x); 
}

Coord indexToPixel(int i) { 
	Coord c = {(i % GRID_WIDTH) * (SCREEN_WIDTH / GRID_WIDTH), (i / GRID_WIDTH) * (SCREEN_HEIGHT / GRID_HEIGHT)};
	return c;
}

Coord indexToCoord(int i) {
	Coord c = {(i % GRID_WIDTH), (GRID_WIDTH - 1 - (i / GRID_WIDTH))};
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
	Grid::speed = INITIAL_SPEED;
	// initialize world
	for (int i = 0; i < (Grid::width * Grid::height); i++) {
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
		delete temp;
		setObject(coordToIndex(coord), snake);
		snakes.push_back(snake);
	}
	// add fruit
	for (int i = 0; i < MAX_FRUIT; i++) {
		addFruit();
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

void Grid::resetHasMoved() {
	for (auto i = Grid::snakes.begin(); i != Grid::snakes.end(); i++) {
		(static_cast<Snake*>(*i))->resetMoveStatus();
	}
}

void Grid::addSnakeBodies() {
	for (auto i = snakes.begin(); i != snakes.end(); i++) {
		Snake* snake = static_cast<Snake*>(*i);
		vector<Coord> body = snake->getBodyVector();
		sf::Color color = snake->getColor();
		for (auto j = body.begin(); j != body.end(); j++) {
			Object* bodySegment = new SnakeBody(coordToIndex(*j), color);
			Object* temp = Grid::getObject(coordToIndex(*j));
			delete temp;
			Grid::setObject(coordToIndex(*j), bodySegment);
		}
	}
}

void Grid::replenishFruit() {
	// If there are less than max fruit, there is a chance to add a fruit
	// The fewer fruits on the grid, the higher chance of adding a new one
	if (fruits.size() < MAX_FRUIT && rand() % static_cast<int>((1.0 / pow(0.5, fruits.size()))) == 0) {
		addFruit();
	}
}

void Grid::addFruit() {
	int fruit_index;
	do {
		fruit_index = rand() % (GRID_WIDTH * GRID_HEIGHT);
	} while ( Grid::getObject(fruit_index)->whatAmI() != EMPTY );
	Object* fruit = new Fruit(indexToCoord(fruit_index));
	Object* temp = getObject(fruit_index);
	delete temp;
	setObject(fruit_index, fruit);
	fruits.push_back(fruit);
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
	Snake::hasMovedYet = false;
	Snake::fruitsEaten = 0;
	switch (Snake::playerID) {
		case 0:
			Snake::color = sf::Color(154, 205, 50); // yellowgreen
			Snake::direction = DOWN;
			Snake::newDirection = DOWN;
			break;
		case 1:
			Snake::color = sf::Color(85, 107, 47); // darkolivegreen
			Snake::direction = UP;
			Snake::newDirection = UP;
			break;
		case 2:
			Snake::color = sf::Color(0, 100, 0); // darkgreen
			Snake::direction = RIGHT;
			Snake::newDirection = RIGHT;
			break;
		case 3:
			Snake::color = sf::Color(46, 139, 87); // seagreen
			Snake::direction = LEFT;
			Snake::newDirection = LEFT;
			break;
	}
}

void Snake::draw(sf::RenderWindow& window, int index) {
	Coord c = indexToPixel(index);
	sf::RectangleShape rectangle(sf::Vector2f(SCREEN_WIDTH / GRID_WIDTH, SCREEN_HEIGHT / GRID_HEIGHT));
	rectangle.setPosition(c.x,c.y);
	rectangle.setFillColor(Snake::color);
	window.draw(rectangle);
}

Result Snake::move(Grid& world) {
	if (Snake::hasMovedYet == true) {
		Snake::hasMovedYet = false;
		return FAILURE;
	}
	Snake::hasMovedYet = true;
	Coord new_location = Snake::getLocation();
	switch (Snake::newDirection) {
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
	if (locationIsInvalid(new_location))  {
		return static_cast<Result>(Snake::playerID + 1);
	}
	Obj collisionObject = world.getObject(coordToIndex(new_location))->whatAmI();
	if (collisionObject != FRUIT && collisionObject != EMPTY)  {
		return static_cast<Result>(Snake::playerID + 1);
	}
	// If snake consumes fruit:
	if (world.getObject(coordToIndex(new_location))->whatAmI() == FRUIT) {
		Snake::body.push_back(Snake::getLocation());
		Object* empty = new Empty(coordToIndex(new_location));
		Object* temp = world.getObject(coordToIndex(new_location));
		delete temp;
		world.setObject(coordToIndex(new_location), empty);
		Snake::fruitsEaten++;
		// speed up game
		world.setSpeed(world.getSpeed() * SPEED_INCREASE_RATE);
	}
	else if (Snake::body.size() > 0) { // If no fruit is comsumed, adjust body
		Snake::body.erase(body.begin());
		Snake::body.push_back(Snake::getLocation());
	}
	world.swapObjects(coordToIndex(Snake::getLocation()), coordToIndex(new_location));
	Snake::direction = Snake::newDirection;
	return SUCCESS;
}

void Fruit::draw(sf::RenderWindow& window, int index) {
	Coord c = indexToPixel(index);
	sf::RectangleShape rectangle(sf::Vector2f(SCREEN_WIDTH / GRID_WIDTH, SCREEN_HEIGHT / GRID_HEIGHT));
	rectangle.setPosition(c.x,c.y);
	rectangle.setFillColor(Fruit::color);
	window.draw(rectangle);
}
