#include <iostream>
#include <memory>
#include <string>
#include <fmt/format.h>

// Include our headers
#include "core/Types.h"
#include "core/Board.h"
#include "game/GameManager.h"
#include "game/Player.h"
#include "ui/Gui.h" // Include the new GUI class

using namespace Chess;

// Main entry point for the application.
int main() {
    try {
        // Setup the game configuration
        GameConfig config;
        config.mode = GameMode::HUMAN_VS_AI;
        config.whitePlayer.isHuman = true;
        config.whitePlayer.name = "Human Player";
        config.blackPlayer.isHuman = false;
        config.blackPlayer.name = "ChessBot AI";
        config.blackPlayer.difficulty = Difficulty::EASY;
        config.useGui = true;

        // Create the GameManager
        GameManager gameManager(config);

        // Create the GUI instance and pass the GameManager
        Gui gui(gameManager);

        // Run the main game loop
        gui.run();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
