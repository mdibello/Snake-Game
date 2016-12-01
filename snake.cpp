// Matthew DiBello
// Snake Game

#include <iostream>
#include <vector>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;

const int GRID_WIDTH = 10;
const int GRID_HEIGHT = 10;
const int SCREEN_WIDTH = 900;
const int SCREEN_HEIGHT = 900;

struct coord {
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
		Object* getTile(coord xy);
		void setTile(coord xy, Object* object);
		int getHeight() { return height; }
		int getWidth() { return width; }
		int getSize() { return grid.size(); }
	private:
		int coordToInt(coord c) { return (((height - 1 - c.y) * width) + c.x); }
		vector<Object*> grid;
		int height;
		int width;
};

class Object {
	public:
	private:
};

void render(sf::RenderWindow& window, Grid world);

int main() {
	
	sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Matt DiBello's Snake Game", 
		sf::Style::Titlebar | sf::Style::Close);
	// sync application refresh rate with monitor
	window.setVerticalSyncEnabled(true);
	// limit refresh rate to 60 fps
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);

	window.clear(sf::Color(0, 0, 255));
	window.display();
	
	Grid world(GRID_WIDTH, GRID_HEIGHT);

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
	window.clear(sf::Color::Black);
	
	window.display();
}
