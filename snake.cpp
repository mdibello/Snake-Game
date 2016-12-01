// Matthew DiBello
// Snake Game

#include <iostream>
#include <SFML/Window.hpp>

using namespace std;

int main() {
	
	sf::Window window(sf::VideoMode(800, 600), "My Window");
	// sync application refresh rate with monitor
	window.setVerticalSyncEnabled(true);
	// limit refresh rate to 60 fps
	window.setFramerateLimit(60);
	
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
				case sf::Event::LostFocus:
					// resume?
					break;
				case sf::Event::KeyPressed:
					// something
					break;
				default:
					break;
			}

		}
	}

	return 0;

}
