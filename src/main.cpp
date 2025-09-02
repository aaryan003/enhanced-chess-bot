#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

#include <iostream>

#include "core/Types.h"
#include "core/Board.h"
#include "game/GameManager.h"
#include "game/Player.h"
#include "ui/Gui.h"

using namespace Chess;

int main() {
    std::cout << "Enhanced Chess Bot - Professional Edition v2.0\n";
    std::cout << "Initializing SFML window...\n";

    // 1) Create SFML window
    sf::RenderWindow window(sf::VideoMode({ 1024,768 }), "Chess Bot");
    window.setFramerateLimit(60);

    // 2) Initialize ImGui-SFML (creates ImGui context internally)
    if (!ImGui::SFML::Init(window)) {
        std::cerr << "[ERROR] ImGui-SFML Init failed\n";
        return 1;
    }
    std::cout << "[INFO] ImGui-SFML initialized successfully\n";

    ImGui::StyleColorsDark(); // safe here: context exists

    // 3) Construct GameManager and Gui
    GameConfig config;
    GameManager gameManager(config);
    Gui gui(gameManager, window);

    // 4) Run gui loop (Gui::run calls ImGui::SFML::Update/Render each frame)
    gui.run();

    // 5) Shutdown ImGui-SFML
    ImGui::SFML::Shutdown();
    return 0;
}
