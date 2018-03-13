#include <SFML/Window.hpp>
#include <cmath>
#include <cstdint>
#include <iostream>

int main() {
  sf::Window window(sf::VideoMode(800, 600), "My window");
  window.setFramerateLimit(60);
  window.setPosition(sf::Vector2i(10, 50));
  window.setSize(sf::Vector2u(640, 480));
  window.setTitle("SFML window");

  uint64_t eid = 0;
  while (window.isOpen()) {
    sf::Event event;
    while (window.waitEvent(event)) {
      std::cout << "got event: " << eid << std::endl;
      eid += 1;
      if (event.type == sf::Event::Closed) {
        window.close();
      }
    }
  }

  return 0;
}
