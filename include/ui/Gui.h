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
#include <imgui.h>
#include <imgui-SFML.h>


namespace Chess {

    // Enums for GUI settings
    enum class BoardTheme {
        CLASSIC,
        DARK,
        BLUE,
        GREEN,
        PURPLE,
        MARBLE
    };

    enum class PieceSet {
        CLASSIC,
        MODERN,
        STAUNTON,
        MEDIEVAL
    };

    enum class GuiState {
        MAIN_MENU,
        GAME_SETUP,
        PLAYING,
        PAUSED,
        GAME_OVER,
        ANALYSIS,
        SETTINGS
    };

    // Structure for board themes
    struct ThemeColors {
        sf::Color lightSquare;
        sf::Color darkSquare;
        sf::Color highlightColor;
        sf::Color moveHighlight;
        sf::Color lastMoveHighlight;
        sf::Color checkHighlight;
        sf::Color borderColor;
        std::string name;
    };

    // Structure for move arrows
    struct MoveArrow {
        Position from;
        Position to;
        sf::Color color;
        float thickness;
    };

    // Settings structure
    struct GuiSettings {
        BoardTheme boardTheme = BoardTheme::CLASSIC;
        PieceSet pieceSet = PieceSet::CLASSIC;
        bool showCoordinates = true;
        bool showLastMove = true;
        bool showPossibleMoves = true;
        bool enableSounds = true;
        bool showMoveArrows = true;
        bool enableAnimations = true;
        float pieceScale = 0.85f;
        float boardRotation = 0.0f;
        int windowWidth = 1200;
        int windowHeight = 900;
        bool showEngineAnalysis = true;
        bool showMoveHistory = true;
        bool showGameInfo = true;
        int engineDepth = 10;
        float evaluationBarWidth = 20.0f;
        bool showEvaluationBar = true;
    };

    /**
     * @class Gui
     * @brief Advanced GUI manager with ImGui integration and professional features
     */
    class Gui {
    private:
        // Core components
        sf::RenderWindow window;
        GameManager& gameManager;
        sf::Clock deltaClock;
        GuiState currentState = GuiState::MAIN_MENU;
        GuiSettings settings;

        // Board rendering properties
        float boardSize = 640.0f;
        float squareSize = boardSize / BOARD_SIZE;
        sf::Vector2f boardOffset = {50.0f, 50.0f};

        // Textures and sprites
        std::map<PieceType, std::map<Color, sf::Sprite>> pieceSprites;
        std::vector<ThemeColors> themes;

        // UI panels and states
        bool showMainMenu = true;
        bool showGameSetup = false;
        bool showAbout = false;

        // Game state
        bool isPaused = false;
        bool isAnalysisMode = false;
        Position selectedPosition = {-1, -1};
        std::vector<Position> possibleMoves;
        std::vector<MoveArrow> moveArrows;
        Position lastMoveFrom = {-1, -1};
        Position lastMoveTo = {-1, -1};
        std::vector<MoveHistoryEntry> tempMoveHistory;
        std::stack<Move> redoStack;


        // Drag and drop
        bool isDragging = false;
        sf::Vector2f dragOffset;
        sf::Vector2i mousePos;
        sf::Sprite* draggedSprite = nullptr;

        // Animation system
        struct PieceAnimation {
            Position from;
            Position to;
            sf::Vector2f currentPos;
            sf::Vector2f startPos;
            sf::Vector2f endPos;
            float progress;
            float duration;
            bool active;
            PieceType pieceType;
            Color pieceColor;
        };
        PieceAnimation currentAnimation;

        // Engine analysis data
        struct EngineEvaluation {
            float score = 0.0f;
            std::string bestMove = "";
            int depth = 0;
            std::vector<std::string> principalVariation;
            int nodes = 0;
            int time = 0;
        };
        EngineEvaluation currentEvaluation;

        // Private methods - Core rendering
        void initializeThemes();
        void loadTextures();
        void loadPieceTextures();
        void createDefaultPieceTextures();

        // Rendering methods
        void renderGame();
        void renderBoard();
        void renderCoordinates();
        void renderPieces();
        void renderHighlights();
        void renderMoveArrows();
        void renderLastMove();
        void renderDraggedPiece();
        void renderEvaluationBar();
        void renderGamePanels();

        // ImGui panels
        void renderMainMenu();
        void renderGameSetup();
        void renderGameControls();
        void renderMoveHistory();
        void renderEngineAnalysis();
        void renderGameInfo();
        void renderSettings();
        void renderAboutDialog();
        void renderMenuBar();

        // Event handling
        void handleMouseEvents(const sf::Event& event);
        void handleKeyboardEvents(const sf::Event& event);

        // Game logic helpers
        void makeMove(const Position& from, const Position& to, PieceType promotion = PieceType::EMPTY);
        void undoMove();
        void redoMove();
        void resetGame();
        void pauseGame();
        void resumeGame();
        void enterAnalysisMode();
        void exitAnalysisMode();

        // Coordinate conversion
        Position getBoardPosition(const sf::Vector2i& mousePos) const;
        sf::Vector2f getScreenPosition(const Position& boardPos) const;
        bool isPositionOnBoard(const sf::Vector2i& mousePos) const;

        // Move validation and highlighting
        void updatePossibleMoves();
        void addMoveArrow(const Position& from, const Position& to, const sf::Color& color = sf::Color::Yellow);
        void clearMoveArrows();

        // Animation system
        void startPieceAnimation(const Position& from, const Position& to, PieceType piece, Color color);
        void updateAnimation(float deltaTime);

        // Theme and appearance
        void applyTheme(BoardTheme theme);
        sf::Color getSquareColor(int x, int y) const;
        void updateBoardLayout();

        // Settings management
        void saveSettings();
        void loadSettings();
        void applySettings();

        // Utility methods
        std::string positionToString(const Position& pos) const;
        std::string moveToString(const Move& move) const;
        sf::Color blendColors(const sf::Color& a, const sf::Color& b, float factor) const;
        void centerWindow();

    public:
        Gui(GameManager& manager);
        ~Gui();
        void run();
        void setState(GuiState state);
        GuiState getState() const { return currentState; }
        void updateEngineEvaluation(float score, const std::string& bestMove, int depth);
        void showNotification(const std::string& message, float duration = 3.0f);
    };

} // namespace Chess
