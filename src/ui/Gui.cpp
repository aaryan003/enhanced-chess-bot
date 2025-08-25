#include "ui/Gui.h"
#include <fmt/format.h>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

namespace Chess {

Gui::Gui(GameManager& manager) : gameManager(manager) {
    unsigned int size = static_cast<unsigned int>(boardSize);
    window.create(sf::VideoMode({size, size}), "Enhanced Chess Bot");
    window.setFramerateLimit(60);
    loadTextures();
}

Gui::~Gui() {}

void Gui::run() {
    // We will get rid of the console output and test functions from main.cpp
    // and replace them with this SFML game loop.
    gameManager.StartGame();

    while (window.isOpen()) {
        while (std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            // Handle mouse events for drag-and-drop
            if (const auto* mouseButtonPressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                    sf::Vector2i mousePosition(static_cast<int>(mouseButtonPressed->position.x),
                                             static_cast<int>(mouseButtonPressed->position.y));
                    Position boardPos = getBoardPosition(mousePosition);

                    if (boardPos.IsValid()) {
                        const Piece& piece = gameManager.GetBoard().GetPiece(boardPos);
                        if (!piece.IsEmpty() && piece.color == gameManager.GetCurrentPlayer()) {
                            isDragging = true;
                            selectedPosition = boardPos;
                            sf::Vector2f screenPos = getScreenPosition(boardPos);
                            dragOffset = sf::Vector2f(static_cast<float>(mousePosition.x) - screenPos.x,
                                                      static_cast<float>(mousePosition.y) - screenPos.y);
                        }
                    }
                }
            }

            if (const auto* mouseButtonReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mouseButtonReleased->button == sf::Mouse::Button::Left && isDragging) {
                    isDragging = false;
                    sf::Vector2i mousePosition(static_cast<int>(mouseButtonReleased->position.x),
                                             static_cast<int>(mouseButtonReleased->position.y));
                    Position targetPosition = getBoardPosition(mousePosition);

                    if (selectedPosition.IsValid() && targetPosition.IsValid()) {
                        Move move(selectedPosition, targetPosition);
                        gameManager.MakeMove(move);
                    }
                }
            }

            if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                mousePos = sf::Vector2i(static_cast<int>(mouseMoved->position.x),
                                       static_cast<int>(mouseMoved->position.y));
            }
        }

        window.clear();
        renderBoard();
        renderPieces();
        window.display();
    }
}

void Gui::loadTextures() {
    pieceTextures.resize(12); // 6 pieces * 2 colors

    // Create simple colored rectangles as placeholder textures
    for (size_t i = 0; i < 12; ++i) {
        unsigned int size = static_cast<unsigned int>(squareSize);

        // Different colors for different pieces
        sf::Color pieceColor;
        if (i < 6) { // White pieces
            pieceColor = sf::Color(255, 255, 255, 200); // Semi-transparent white
        } else { // Black pieces
            pieceColor = sf::Color(50, 50, 50, 200); // Semi-transparent dark
        }

        // Create image with specified size and color - SFML 3.0 constructor
        sf::Image image({size, size}, pieceColor);

        if (!pieceTextures[i].loadFromImage(image)) {
            std::cerr << "Failed to load texture for piece " << i << std::endl;
        }
    }

    // Initialize white pieces sprites - FIXED: Use emplace to construct in place
    whitePieces.emplace(PieceType::PAWN, sf::Sprite(pieceTextures[0]));
    whitePieces.emplace(PieceType::ROOK, sf::Sprite(pieceTextures[1]));
    whitePieces.emplace(PieceType::KNIGHT, sf::Sprite(pieceTextures[2]));
    whitePieces.emplace(PieceType::BISHOP, sf::Sprite(pieceTextures[3]));
    whitePieces.emplace(PieceType::QUEEN, sf::Sprite(pieceTextures[4]));
    whitePieces.emplace(PieceType::KING, sf::Sprite(pieceTextures[5]));

    // Initialize black pieces sprites - FIXED: Use emplace to construct in place
    blackPieces.emplace(PieceType::PAWN, sf::Sprite(pieceTextures[6]));
    blackPieces.emplace(PieceType::ROOK, sf::Sprite(pieceTextures[7]));
    blackPieces.emplace(PieceType::KNIGHT, sf::Sprite(pieceTextures[8]));
    blackPieces.emplace(PieceType::BISHOP, sf::Sprite(pieceTextures[9]));
    blackPieces.emplace(PieceType::QUEEN, sf::Sprite(pieceTextures[10]));
    blackPieces.emplace(PieceType::KING, sf::Sprite(pieceTextures[11]));

    // Set the scale for all sprites to fit the squares properly
    float scale = 0.8f; // Make pieces slightly smaller than squares
    sf::Vector2f scaleVector(scale, scale);
    for (auto& pair : whitePieces) {
        pair.second.setScale(scaleVector);
    }
    for (auto& pair : blackPieces) {
        pair.second.setScale(scaleVector);
    }
}

void Gui::renderBoard() {
    sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            // FIXED: Use sf::Vector2f for setPosition in SFML 3.0
            square.setPosition(sf::Vector2f(static_cast<float>(x * squareSize),
                                           static_cast<float>(y * squareSize)));
            if ((x + y) % 2 == 0) {
                square.setFillColor(sf::Color(240, 217, 181)); // Light square
            } else {
                square.setFillColor(sf::Color(181, 136, 99)); // Dark square
            }
            window.draw(square);
        }
    }
}

void Gui::renderPieces() {
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            const Piece& piece = gameManager.GetBoard().GetPiece(x, y);
            if (!piece.IsEmpty() && !(isDragging && selectedPosition.x == x && selectedPosition.y == y)) {
                sf::Sprite* spritePtr = nullptr;
                if (piece.color == Color::WHITE) {
                    auto it = whitePieces.find(piece.type);
                    if (it != whitePieces.end()) {
                        spritePtr = &(it->second);
                    }
                } else {
                    auto it = blackPieces.find(piece.type);
                    if (it != blackPieces.end()) {
                        spritePtr = &(it->second);
                    }
                }

                if (spritePtr) {
                    sf::Vector2f pos = getScreenPosition(Position(x, y));
                    // Center the piece in the square
                    float offset = (squareSize * (1.0f - 0.8f)) / 2.0f; // 0.8f is the scale factor
                    spritePtr->setPosition(sf::Vector2f(pos.x + offset, pos.y + offset));
                    window.draw(*spritePtr);
                }
            }
        }
    }

    if (isDragging) {
        drawSelectedPiece();
    }
}

void Gui::drawSelectedPiece() {
    const Piece& piece = gameManager.GetBoard().GetPiece(selectedPosition);
    if (!piece.IsEmpty()) {
        sf::Sprite* spritePtr = nullptr;
        if (piece.color == Color::WHITE) {
            auto it = whitePieces.find(piece.type);
            if (it != whitePieces.end()) {
                spritePtr = &(it->second);
            }
        } else {
            auto it = blackPieces.find(piece.type);
            if (it != blackPieces.end()) {
                spritePtr = &(it->second);
            }
        }

        if (spritePtr) {
            // Center the dragged piece on the mouse cursor
            float halfSize = (squareSize * 0.8f) / 2.0f;
            spritePtr->setPosition(sf::Vector2f(static_cast<float>(mousePos.x) - dragOffset.x - halfSize,
                                              static_cast<float>(mousePos.y) - dragOffset.y - halfSize));
            window.draw(*spritePtr);
        }
    }
}

Position Gui::getBoardPosition(const sf::Vector2i& mousePosition) const {
    int x = static_cast<int>(static_cast<float>(mousePosition.x) / squareSize);
    int y = static_cast<int>(static_cast<float>(mousePosition.y) / squareSize);
    return Position(x, y);
}

sf::Vector2f Gui::getScreenPosition(const Position& boardPos) const {
    return sf::Vector2f(static_cast<float>(boardPos.x * squareSize),
                       static_cast<float>(boardPos.y * squareSize));
}

} // namespace Chess