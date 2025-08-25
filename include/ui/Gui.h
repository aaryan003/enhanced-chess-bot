#pragma once

#include "game/GameManager.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <map>  // Added this missing include

namespace Chess {

    /**
     * @class Gui
     * @brief Manages the graphical user interface for the chess game using SFML.
     */
    class Gui {
    private:
        sf::RenderWindow window;
        GameManager& gameManager;

        // Board rendering properties
        const float boardSize = 800.0f;
        const float squareSize = boardSize / BOARD_SIZE;
        std::vector<sf::Texture> pieceTextures;
        std::map<PieceType, sf::Sprite> whitePieces;
        std::map<PieceType, sf::Sprite> blackPieces;

        // Board state for drag-and-drop
        bool isDragging = false;
        sf::Vector2f dragOffset;
        sf::Vector2i mousePos;
        sf::Vector2i clickPos;
        Position selectedPosition;

        // Helper functions for rendering
        void loadTextures();
        void renderBoard();
        void renderPieces();
        void drawSelectedPiece();

        // Helper function for converting coordinates
        Position getBoardPosition(const sf::Vector2i& mousePos) const;
        sf::Vector2f getScreenPosition(const Position& boardPos) const;

    public:
        /**
         * @brief Constructor for the GUI.
         * @param manager A reference to the GameManager instance.
         */
        Gui(GameManager& manager);
        ~Gui();

        /**
         * @brief Runs the main game loop.
         */
        void run();
    };

} // namespace Chess