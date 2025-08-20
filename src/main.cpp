#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>
#include <fmt/format.h>
#include <iostream>

int main() {
    // Test SFML (note: use {width, height} for VideoMode in SFML 3)
    sf::RenderWindow window(sf::VideoMode({400u, 300u}), "vcpkg Test");

    // Test nlohmann-json
    nlohmann::json j;
    j["test"] = "vcpkg working!";

    // Test fmt
    std::string message = fmt::format("Setup successful! JSON: {}", j.dump());
    std::cout << message << std::endl;

    // Simple SFML window test
    while (window.isOpen()) {
        // pollEvent returns std::optional<sf::Event>
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color::Green);
        window.display();
    }

    return 0;
}
