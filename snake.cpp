// Matthew DiBello
// Snake Game

#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

using namespace std;

const int GRID_WIDTH = 10;
const int SCREEN_WIDTH = 900;

void render(sf::RenderWindow& window);

int main() {
	
	sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_WIDTH), 
		"My Window", sf::Style::Titlebar | sf::Style::Close);
	// sync application refresh rate with monitor
	window.setVerticalSyncEnabled(true);
	// limit refresh rate to 60 fps
	window.setFramerateLimit(60);
	window.setKeyRepeatEnabled(false);

	window.clear(sf::Color(0, 0, 255));
	window.display();
	
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
					render(window);
					break;
				default:
					break;
			}

		}
	}

	return 0;

}

void render(sf::RenderWindow& window) {

	window.clear(sf::Color::Black);

	window.display();


}
