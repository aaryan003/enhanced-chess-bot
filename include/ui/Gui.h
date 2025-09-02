// Gui.h
#pragma once

#include "game/GameManager.h"
#include "core/Board.h"
#include "game/Player.h"
#include "core/Types.h"

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <imgui.h>
#include <imgui-SFML.h>

#include <memory>
#include <vector>
#include <map>
#include <stack>
#include <string>
#include <array>
#include <optional>

namespace Chess {

    // Forward declare enums to use them in GuiSettings
    enum class BoardTheme;
    enum class PieceSet;

    class Gui {
    public:
        // ---- GUI State Machine ----
        enum class GuiState {
            MAIN_MENU,
            GAME_SETUP,
            PLAYING,
            PAUSED,
            GAME_OVER,
            ANALYSIS,
            SETTINGS
        };

        // ---- Enums for Settings ----
        enum class BoardTheme { CLASSIC, DARK, BLUE, GREEN, PURPLE, MARBLE };
        enum class PieceSet { CLASSIC, MODERN, STAUNTON, MEDIEVAL };

        struct GuiSettings {
            int windowWidth = 1280;
            int windowHeight = 800;
            int engineDepth = 10;
            float evaluationBarWidth = 30.0f;

            bool showEngineAnalysis = true;
            bool showMoveHistory = true;
            bool showGameInfo = true;
            bool showEvaluationBar = true;

            float pieceScale = 1.0f;
            float boardRotation = 0.0f;

            bool showCoordinates = true;
            bool showLastMove = true;
            bool showPossibleMoves = true;
            bool showMoveArrows = true;
            bool enableAnimations = true;
            bool enableSounds = true;

            BoardTheme boardTheme = BoardTheme::CLASSIC;
            PieceSet pieceSet = PieceSet::CLASSIC;
        };

        struct EngineEvaluation {
            int depth = 0;
            int nodes = 0;
            float score = 0.0f;
            std::string bestMove;
            std::vector<std::string> principalVariation;
        };

        // ---- Constructor / Destructor ----
        Gui(GameManager& gameManager, sf::RenderWindow& window);
        ~Gui();

        // ---- Main Loop ----
        void run();

        // ---- Render Functions ----
        void renderMenuBar();
        void renderMainMenu();
        void renderGameSetup();
        void renderGameControls();
        void renderMoveHistory();
        void renderEngineAnalysis();
        void renderGameInfo();
        void renderEvaluationBar();
        void renderSettings();
        void renderAboutDialog();

        // ---- State Management ----
        void setState(GuiState state);

    private:
        // ---- Core Components ----
        GameManager& gameManager;
        sf::RenderWindow& window;

        // ---- GUI State ----
        GuiState currentState;
        bool isPaused = false;
        bool isAnalysisMode = false;
        bool showAbout = false;

        GuiSettings settings;
        EngineEvaluation currentEvaluation;

        // ---- Board Layout ----
        sf::Vector2f boardOffset;
        float boardSize;
        float squareSize;
        static constexpr int BOARD_SIZE = 8; // Define board dimensions

        // ---- Input & Interaction State ----
        bool isDragging = false;
        Position selectedPosition = Position(-1, -1);
        sf::Vector2f dragOffset;
        sf::Vector2i mousePos;
        std::vector<Position> possibleMoves;
        Position lastMoveFrom = Position(-1, -1);
        Position lastMoveTo = Position(-1, -1);

        // ---- Animation ----
        struct AnimationState {
            bool active = false;
            float duration = 0.2f; // Adjusted for a slightly faster animation
            float progress = 0.0f;
            Position from, to;
            sf::Vector2f startPos, endPos, currentPos;
            PieceType pieceType;
            Color pieceColor;
        } currentAnimation;

        // ---- Graphics & Theming ----
        struct ThemeColors {
            sf::Color lightSquare;
            sf::Color darkSquare;
            sf::Color selectedSquare;
            sf::Color possibleMove;
            sf::Color lastMove;
            sf::Color checkHighlight;
            sf::Color borderColor;
            std::string name;
        };
        std::vector<ThemeColors> themes;
        std::map<PieceType, std::map<Color, sf::Sprite>> pieceSprites;

        // ---- Move Arrows & History ----
        struct MoveArrow {
            Position from, to;
            sf::Color color;
            float thickness;
        };
        std::vector<MoveArrow> moveArrows;
        std::stack<Move> redoStack;

        // ---- Event Handling ----
        void handleMouseEvents(const sf::Event& event);
        void handleKeyboardEvents(const sf::Event& event);

        // ---- Game Action Helpers ----
        void makeMove(const Position& from, const Position& to, PieceType promotion = PieceType::QUEEN);
        void undoMove();
        void redoMove();
        void resetGame();
        void pauseGame();
        void resumeGame();
        void enterAnalysisMode();
        void exitAnalysisMode();

        // ---- Rendering Helpers ----
        void renderGame();
        void renderGamePanels();
        void renderBoard();
        void renderCoordinates();
        void renderHighlights();
        void renderLastMove();
        void renderMoveArrows();
        void renderPieces();
        void renderDraggedPiece();

        // ---- Coordinate & Position Helpers ----
        Position getBoardPosition(const sf::Vector2i& mousePos) const;
        sf::Vector2f getScreenPosition(const Position& boardPos) const;
        bool isPositionOnBoard(const sf::Vector2i& mousePos) const;

        // ---- Move Generation & Drawing ----
        void updatePossibleMoves();
        void addMoveArrow(const Position& from, const Position& to, const sf::Color& color = sf::Color(255, 165, 0, 150));
        void clearMoveArrows();

        // ---- Animation ----
        void startPieceAnimation(const Position& from, const Position& to, PieceType piece, Color color);
        void updateAnimation(float deltaTime);

        // ---- Initialization & Configuration ----
        void initializeThemes();
        void applyTheme(BoardTheme theme);
        void loadTextures();
        void loadPieceTextures();
        void createDefaultPieceTextures();
        void updateBoardLayout();

        // ---- Settings Management ----
        void loadSettings();
        void saveSettings();
        void applySettings();

        // ---- Utility ----
        sf::Color getSquareColor(int x, int y) const;
        std::string positionToString(const Position& pos) const;
        std::string moveToString(const Move& move) const;
        sf::Color blendColors(const sf::Color& a, const sf::Color& b, float factor) const;
        void centerWindow();
        void updateEngineEvaluation();
        void showNotification(const std::string& message);
    };

} // namespace Chess