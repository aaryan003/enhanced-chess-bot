#include "ui/Gui.h"
#include <imgui.h>
#include <imgui-SFML.h>
#include <fmt/format.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <filesystem>
#include <chrono>


namespace Chess {

    Gui::Gui(GameManager& manager, sf::RenderWindow& win)
        : window(win), gameManager(manager) {
        // Basic defaults
        currentState = GuiState::MAIN_MENU;
        settings = GuiSettings{};
        boardOffset = { 50.f, 50.f };
        boardSize = 640.0f;
        squareSize = boardSize / 8.0f;

        // Load settings (optional) - don't recreate window here
        loadSettings();

        // Compute layout based on possibly-updated settings
        updateBoardLayout();

        // Safe to call ImGui styling here because main called ImGui::SFML::Init(window)

        // Load assets and theme
        initializeThemes();
        loadTextures();
        applyTheme(settings.boardTheme);

        currentAnimation.active = false;
        currentAnimation.duration = 0.3f;
    }

    Gui::~Gui() {
        // Save settings, but do NOT call ImGui::SFML::Shutdown() here (main is responsible)
        saveSettings();
    }

    void Gui::run() {
        sf::Clock deltaClock;

        while (window.isOpen()) {
            // --- 1. Process Events ---
            while (auto eventOpt = window.pollEvent()) {
                const auto& event = *eventOpt;

                // Pass event to ImGui
                ImGui::SFML::ProcessEvent(window, event);

                // Pass to game-specific handlers
                handleMouseEvents(event);
                handleKeyboardEvents(event);

                if (event.is<sf::Event::Closed>()) {
                    window.close();
                }
            }

            // --- 2. Update ImGui ---
            ImGui::SFML::Update(window, deltaClock.restart());
            ImGui::StyleColorsDark();
            // --- 3. Draw ---
            window.clear(sf::Color(24, 24, 24));

            if (currentState != GuiState::MAIN_MENU && currentState != GuiState::GAME_SETUP) {
                renderGame(); // background chessboard & pieces
            }

            renderMenuBar();

            switch (currentState) {
            case GuiState::MAIN_MENU:   renderMainMenu(); break;
            case GuiState::GAME_SETUP:  renderGameSetup(); break;
            case GuiState::PLAYING:
            case GuiState::PAUSED:
            case GuiState::GAME_OVER:
            case GuiState::ANALYSIS:    renderGamePanels(); break;
            case GuiState::SETTINGS:    renderSettings(); break;
            }

            if (showAbout) {
                renderAboutDialog();
            }

            // --- 4. Render ImGui ---
            ImGui::SFML::Render(window);

            window.display();
        }
    }


    //void Gui::renderMainMenu() {
    //    ImGui::SetNextWindowPos(ImVec2(settings.windowWidth * 0.5f, settings.windowHeight * 0.5f),
    //        ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    //    ImGui::SetNextWindowSize(ImVec2(400, 500));

    //    bool open = ImGui::Begin("Enhanced Chess Bot", nullptr,
    //        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    //    if (open) {
    //        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Enhanced Chess Bot").x) * 0.5f);
    //        ImGui::Text("Enhanced Chess Bot");

    //        ImGui::Separator();
    //        ImGui::Spacing();

    //        if (ImGui::Button("Human vs AI", ImVec2(-1, 50))) {
    //            setState(GuiState::GAME_SETUP);
    //        }

    //        if (ImGui::Button("Human vs Human", ImVec2(-1, 50))) {
    //            gameManager.SetupNewGame(GameConfig{ GameMode::HUMAN_VS_HUMAN });
    //            gameManager.StartGame();
    //            setState(GuiState::PLAYING);
    //        }

    //        if (ImGui::Button("AI vs AI", ImVec2(-1, 50))) {
    //            gameManager.SetupNewGame(GameConfig{ GameMode::AI_VS_AI });
    //            gameManager.StartGame();
    //            setState(GuiState::PLAYING);
    //        }

    //        if (ImGui::Button("Analysis Mode", ImVec2(-1, 50))) {
    //            enterAnalysisMode();
    //            setState(GuiState::ANALYSIS);
    //        }

    //        ImGui::Spacing();
    //        ImGui::Separator();
    //        ImGui::Spacing();

    //        if (ImGui::Button("Settings", ImVec2(-1, 40))) {
    //            setState(GuiState::SETTINGS);
    //        }

    //        if (ImGui::Button("About", ImVec2(-1, 40))) {
    //            showAbout = true;
    //        }

    //        if (ImGui::Button("Exit", ImVec2(-1, 40))) {
    //            window.close();
    //        }
    //    }
    //    ImGui::End();

    //    if (showAbout) {
    //        // About dialog is drawn separately by run() if showAbout is true,
    //        // but we keep this here as well so behavior matches earlier code.
    //    }
    //}


    void Gui::renderMainMenu() {
        // Center the window
        ImGui::SetNextWindowPos(ImVec2(settings.windowWidth * 0.5f, settings.windowHeight * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Always);

        // Begin the window. The second argument is a pointer to a boolean to control visibility.
        // The return value of Begin tells us if the window is collapsed or not.
        if (ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
        {
            // --- All your widgets go inside this if-statement ---

            ImGui::Text("Welcome to Enhanced Chess Bot!");
            ImGui::Spacing(); // Adds a little vertical space

            if (ImGui::Button("Start Game", ImVec2(-1, 40))) {
                // We'll add the logic back later
                setState(GuiState::GAME_SETUP);
            }

            if (ImGui::Button("Exit", ImVec2(-1, 40))) {
                window.close();
            }

            // --- End of widget section ---
        }

        // CRUCIAL: You MUST call ImGui::End() to match the ImGui::Begin() call.
        ImGui::End();
    }

    void Gui::renderGameSetup() {
        ImGui::SetNextWindowPos(ImVec2(settings.windowWidth * 0.5f, settings.windowHeight * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(450, 400));

        bool open = ImGui::Begin("Game Setup", nullptr,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        if (open) {
            static char whiteName[64] = "Human Player";
            static char blackName[64] = "ChessBot AI";
            static int difficulty = 1;
            static bool whiteIsHuman = true;
            static bool blackIsHuman = false;
            static int timeControl = 0;

            ImGui::Text("White Player:");
            ImGui::Checkbox("Human##White", &whiteIsHuman);
            ImGui::SameLine();
            ImGui::InputText("Name##White", whiteName, sizeof(whiteName));

            ImGui::Spacing();

            ImGui::Text("Black Player:");
            ImGui::Checkbox("Human##Black", &blackIsHuman);
            ImGui::SameLine();
            ImGui::InputText("Name##Black", blackName, sizeof(blackName));

            if (!blackIsHuman) {
                ImGui::Text("AI Difficulty:");
                const char* difficultyItems[] = { "Beginner", "Easy", "Medium", "Hard", "Expert", "Master", "Grandmaster" };
                ImGui::Combo("##Difficulty", &difficulty, difficultyItems, IM_ARRAYSIZE(difficultyItems));
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Time Control:");
            const char* timeItems[] = { "No Limit", "3+0 Blitz", "5+3 Blitz", "10+0 Rapid", "15+10 Rapid", "30+0 Classical" };
            ImGui::Combo("##TimeControl", &timeControl, timeItems, IM_ARRAYSIZE(timeItems));

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Start Game", ImVec2(-1, 40))) {
                GameConfig config;
                config.mode = whiteIsHuman && blackIsHuman ? GameMode::HUMAN_VS_HUMAN :
                    (!whiteIsHuman && !blackIsHuman) ? GameMode::AI_VS_AI : GameMode::HUMAN_VS_AI;

                config.whitePlayer.isHuman = whiteIsHuman;
                config.whitePlayer.name = whiteName;
                config.blackPlayer.isHuman = blackIsHuman;
                config.blackPlayer.name = blackName;
                config.blackPlayer.difficulty = static_cast<Difficulty>(difficulty);

                // Time control simplified for this sample; keep your original mapping if needed.
                config.whitePlayer.timeControl = config.blackPlayer.timeControl;

                gameManager.SetupNewGame(config);
                gameManager.StartGame();
                setState(GuiState::PLAYING);
            }

            if (ImGui::Button("Back to Main Menu", ImVec2(-1, 40))) {
                setState(GuiState::MAIN_MENU);
            }
        }
        ImGui::End();
    }

    void Gui::renderGame() {
        // Compose the game drawing from smaller functions.
        // (Implement full board/piece drawing in your original code; these are safe stubs.)
        renderBoard();
        renderHighlights();
        renderPieces();
        renderLastMove();
        renderMoveArrows();
        renderDraggedPiece();
    }

    void Gui::renderGamePanels() {
        renderGameControls();
        if (settings.showMoveHistory) renderMoveHistory();
        if (settings.showEngineAnalysis) renderEngineAnalysis();
        if (settings.showGameInfo) renderGameInfo();
        if (settings.showEvaluationBar) renderEvaluationBar();
    }

    void Gui::renderBoard() {

        // stub: actual board drawing should use sf::RectangleShape, textures, etc.
        // For now we do nothing here; your original implementation can be restored.

           /*sf::RectangleShape square(sf::Vector2f(squareSize, squareSize));

           for (int y = 0; y < BOARD_SIZE; ++y) {
               for (int x = 0; x < BOARD_SIZE; ++x) {
                   square.setPosition({ boardOffset.x + x * squareSize,
                                       boardOffset.y + y * squareSize });
                   square.setFillColor(getSquareColor(x, y));
                   window.draw(square);
               }
           }

           sf::RectangleShape border;
           border.setSize(sf::Vector2f(boardSize + 4, boardSize + 4));
           border.setPosition({ boardOffset.x - 2, boardOffset.y - 2 });
           border.setFillColor(sf::Color::Transparent);
           border.setOutlineThickness(2);
           border.setOutlineColor(themes.at(static_cast<int>(settings.boardTheme)).borderColor);
           window.draw(border);*/
    }

    void Gui::renderCoordinates() {
        if (!settings.showCoordinates) return;
    }

    void Gui::renderHighlights() {}

    void Gui::renderLastMove() {}
    void Gui::renderMoveArrows() {}

    void Gui::renderPieces() {}

    void Gui::renderDraggedPiece() {}

    void Gui::renderGameControls() {
        ImGui::SetNextWindowPos(ImVec2(10, 10));
        ImGui::SetNextWindowSize(ImVec2(200, 300));

        bool open = ImGui::Begin("Game Controls", nullptr, ImGuiWindowFlags_NoResize);
        if (open) {
            if (currentState == GuiState::PLAYING) {
                if (isPaused) {
                    if (ImGui::Button("Resume", ImVec2(-1, 30))) resumeGame();
                }
                else {
                    if (ImGui::Button("Pause", ImVec2(-1, 30))) pauseGame();
                }
            }

            if (ImGui::Button("Undo Move", ImVec2(-1, 30))) undoMove();
            if (ImGui::Button("Redo Move", ImVec2(-1, 30))) redoMove();

            ImGui::Separator();

            if (currentState == GuiState::ANALYSIS) {
                if (ImGui::Button("Exit Analysis", ImVec2(-1, 30))) {
                    exitAnalysisMode();
                    setState(GuiState::PLAYING);
                }
            }
            else {
                if (ImGui::Button("Analysis Mode", ImVec2(-1, 30))) {
                    enterAnalysisMode();
                    setState(GuiState::ANALYSIS);
                }
            }

            ImGui::Separator();

            if (ImGui::Button("New Game", ImVec2(-1, 30))) resetGame();
            if (ImGui::Button("Main Menu", ImVec2(-1, 30))) setState(GuiState::MAIN_MENU);

            ImGui::Separator();
            ImGui::Text("Panels:");
            ImGui::Checkbox("Move History", &settings.showMoveHistory);
            ImGui::Checkbox("Engine Analysis", &settings.showEngineAnalysis);
            ImGui::Checkbox("Game Info", &settings.showGameInfo);
            ImGui::Checkbox("Evaluation Bar", &settings.showEvaluationBar);
        }
        ImGui::End();
    }

    void Gui::renderMoveHistory() {
        ImGui::SetNextWindowPos(ImVec2(settings.windowWidth - 250, 10));
        ImGui::SetNextWindowSize(ImVec2(240, 400));

        bool open = ImGui::Begin("Move History", &settings.showMoveHistory);
        if (open) {
            ImGui::Text("Move History:");
            ImGui::Separator();
            const auto& history = gameManager.GetMoveHistory();
            for (const auto& entry : history) {
                ImGui::Text("%d. %s", entry.fullMoveNumber, entry.algebraicNotation.c_str());
            }

            if (ImGui::Button("Export PGN", ImVec2(-1, 30))) {
                // Export functionality goes here (stub)
            }
        }
        ImGui::End();
    }

    void Gui::renderEngineAnalysis() {
        ImGui::SetNextWindowPos(ImVec2(settings.windowWidth - 250, 420));
        ImGui::SetNextWindowSize(ImVec2(240, 300));

        bool open = ImGui::Begin("Engine Analysis", &settings.showEngineAnalysis);
        if (open) {
            ImGui::Text("Depth: %d", currentEvaluation.depth);
            ImGui::Text("Nodes: %d", currentEvaluation.nodes);
            ImGui::Text("Score: %.2f", currentEvaluation.score);

            ImGui::Separator();

            ImGui::Text("Best Move: %s", currentEvaluation.bestMove.c_str());

            ImGui::Separator();

            ImGui::Text("Principal Variation:");
            for (size_t i = 0; i < currentEvaluation.principalVariation.size() && i < 5; ++i) {
                ImGui::Text("%zu. %s", i + 1, currentEvaluation.principalVariation[i].c_str());
            }

            ImGui::Separator();

            ImGui::SliderInt("Engine Depth", &settings.engineDepth, 1, 20);

            if (ImGui::Button("Start Analysis", ImVec2(-1, 30))) {
                // stub: start engine analysis
            }
        }
        ImGui::End();
    }

    void Gui::renderGameInfo() {
        ImGui::SetNextWindowPos(ImVec2(10, 320));
        ImGui::SetNextWindowSize(ImVec2(200, 200));

        bool open = ImGui::Begin("Game Info", &settings.showGameInfo);
        if (open) {
            ImGui::Text("White: %s", gameManager.GetPlayer(Color::WHITE)->GetName().c_str());
            ImGui::Text("Black: %s", gameManager.GetPlayer(Color::BLACK)->GetName().c_str());

            ImGui::Separator();

            ImGui::Text("Turn: %s", gameManager.GetCurrentPlayer() == Color::WHITE ? "White" : "Black");

            GameResult result = gameManager.GetGameResult();
            if (result != GameResult::ONGOING) {
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "GAME OVER");

                switch (result) {
                case GameResult::CHECKMATE_WHITE: ImGui::Text("White wins by checkmate!"); break;
                case GameResult::CHECKMATE_BLACK: ImGui::Text("Black wins by checkmate!"); break;
                case GameResult::STALEMATE: ImGui::Text("Stalemate!"); break;
                case GameResult::DRAW_50_MOVES:
                case GameResult::DRAW_REPETITION:
                case GameResult::DRAW_MATERIAL:
                    ImGui::Text("Draw!"); break;
                case GameResult::TIMEOUT_WHITE: ImGui::Text("Black wins on time!"); break;
                case GameResult::TIMEOUT_BLACK: ImGui::Text("White wins on time!"); break;
                case GameResult::RESIGNATION: ImGui::Text("Resignation!"); break;
                default: break;
                }
            }

            ImGui::Separator();

            ImGui::Text("Move Count: %zu", gameManager.GetMoveCount());
        }
        ImGui::End();
    }
    void Gui::renderEvaluationBar() {
        if (!settings.showEvaluationBar) return;

        ImGui::SetNextWindowPos(ImVec2(boardOffset.x + boardSize + 10, boardOffset.y));
        ImGui::SetNextWindowSize(ImVec2(settings.evaluationBarWidth + 20, boardSize));

        bool open = ImGui::Begin("Evaluation", &settings.showEvaluationBar,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
        if (open) {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
            ImVec2 canvas_sz = ImGui::GetContentRegionAvail();

            float normalizedScore = std::max(-10.0f, std::min(10.0f, currentEvaluation.score));
            float barHeight = canvas_sz.y;
            float whiteHeight = (normalizedScore + 10.0f) / 20.0f * barHeight;

            draw_list->AddRectFilled(
                ImVec2(canvas_p0.x, canvas_p0.y + barHeight - whiteHeight),
                ImVec2(canvas_p0.x + settings.evaluationBarWidth, canvas_p0.y + barHeight),
                IM_COL32(240, 240, 240, 255)
            );

            draw_list->AddRectFilled(
                canvas_p0,
                ImVec2(canvas_p0.x + settings.evaluationBarWidth, canvas_p0.y + barHeight - whiteHeight),
                IM_COL32(60, 60, 60, 255)
            );

            draw_list->AddLine(
                ImVec2(canvas_p0.x, canvas_p0.y + barHeight * 0.5f),
                ImVec2(canvas_p0.x + settings.evaluationBarWidth, canvas_p0.y + barHeight * 0.5f),
                IM_COL32(100, 100, 100, 255), 2.0f
            );
        }
        ImGui::End();
    }
    void Gui::renderSettings() {
        ImGui::SetNextWindowPos(ImVec2(settings.windowWidth * 0.5f, settings.windowHeight * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(500, 600));

        bool open = ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        if (open) {
            if (ImGui::BeginTabBar("SettingsTabs")) {
                if (ImGui::BeginTabItem("Appearance")) {
                    ImGui::Text("Board Theme:");
                    const char* themeNames[] = { "Classic", "Dark", "Blue", "Green", "Purple", "Marble" };
                    int currentTheme = static_cast<int>(settings.boardTheme);
                    if (ImGui::Combo("##BoardTheme", &currentTheme, themeNames, IM_ARRAYSIZE(themeNames))) {
                        settings.boardTheme = static_cast<BoardTheme>(currentTheme);
                        applyTheme(settings.boardTheme);
                    }

                    ImGui::Text("Piece Set:");
                    const char* pieceSetNames[] = { "Classic", "Modern", "Staunton", "Medieval" };
                    int currentPieceSet = static_cast<int>(settings.pieceSet);
                    if (ImGui::Combo("##PieceSet", &currentPieceSet, pieceSetNames, IM_ARRAYSIZE(pieceSetNames))) {
                        settings.pieceSet = static_cast<PieceSet>(currentPieceSet);
                        loadPieceTextures();
                    }

                    ImGui::SliderFloat("Piece Scale", &settings.pieceScale, 0.5f, 1.0f);
                    ImGui::SliderFloat("Board Rotation", &settings.boardRotation, 0.0f, 360.0f);

                    ImGui::Checkbox("Show Coordinates", &settings.showCoordinates);
                    ImGui::Checkbox("Show Last Move", &settings.showLastMove);
                    ImGui::Checkbox("Show Possible Moves", &settings.showPossibleMoves);
                    ImGui::Checkbox("Show Move Arrows", &settings.showMoveArrows);
                    ImGui::Checkbox("Enable Animations", &settings.enableAnimations);

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Gameplay")) {
                    ImGui::Checkbox("Enable Sounds", &settings.enableSounds);
                    ImGui::SliderInt("Engine Depth", &settings.engineDepth, 1, 20);

                    ImGui::Separator();

                    ImGui::Text("GUI Panels:");
                    ImGui::Checkbox("Show Engine Analysis##Settings", &settings.showEngineAnalysis);
                    ImGui::Checkbox("Show Move History##Settings", &settings.showMoveHistory);
                    ImGui::Checkbox("Show Game Info##Settings", &settings.showGameInfo);
                    ImGui::Checkbox("Show Evaluation Bar##Settings", &settings.showEvaluationBar);
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Display")) {
                    ImGui::SliderInt("Window Width", &settings.windowWidth, 800, 1920);
                    ImGui::SliderInt("Window Height", &settings.windowHeight, 600, 1080);

                    if (ImGui::Button("Apply Window Size", ImVec2(-1, 30))) {
                        window.setSize(sf::Vector2u(settings.windowWidth, settings.windowHeight));
                        updateBoardLayout();
                    }

                    if (ImGui::Button("Center Window", ImVec2(-1, 30))) {
                        centerWindow();
                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::Separator();

            if (ImGui::Button("Apply Settings", ImVec2(-1, 40))) {
                applySettings();
                saveSettings();
            }

            ImGui::SameLine();

            if (ImGui::Button("Reset to Defaults", ImVec2(-1, 40))) {
                settings = GuiSettings{};
                applySettings();
            }

            if (ImGui::Button("Back", ImVec2(-1, 40))) {
                setState(GuiState::MAIN_MENU);
            }
        }
        ImGui::End();
    }
    void Gui::renderAboutDialog() {
        ImGui::SetNextWindowPos(ImVec2(settings.windowWidth * 0.5f, settings.windowHeight * 0.5f),
            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400, 300));

        bool open = ImGui::Begin("About Enhanced Chess Bot", &showAbout,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
        if (open) {
            ImGui::Text("Enhanced Chess Bot");
            ImGui::Text("Professional Edition v2.0");
            ImGui::Separator();

            ImGui::Text("Features:");
            ImGui::BulletText("Advanced AI with multiple difficulty levels");
            ImGui::BulletText("Professional GUI with ImGui integration");
            ImGui::BulletText("Multiple board themes and piece sets");
            ImGui::BulletText("Real-time engine analysis");
            ImGui::BulletText("Move history and game analysis");
            ImGui::BulletText("Drag-and-drop piece movement");
            ImGui::BulletText("Animation and visual effects");
            ImGui::BulletText("Customizable settings and preferences");

            ImGui::Separator();

            ImGui::Text("Built with:");
            ImGui::BulletText("C++ and modern programming practices");
            ImGui::BulletText("SFML 3.0 for graphics and window management");
            ImGui::BulletText("Dear ImGui for advanced GUI components");
            ImGui::BulletText("Custom chess engine implementation");

            if (ImGui::Button("Close", ImVec2(-1, 30))) showAbout = false;
        }
        ImGui::End();
    }
    void Gui::renderMenuBar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Game")) {
                if (ImGui::MenuItem("New Game", "Ctrl+N")) resetGame();
                if (ImGui::MenuItem("Save Game", "Ctrl+S")) { /* save */ }
                if (ImGui::MenuItem("Load Game", "Ctrl+O")) { /* load */ }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) window.close();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) undoMove();
                if (ImGui::MenuItem("Redo", "Ctrl+Y")) redoMove();
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Move History", nullptr, &settings.showMoveHistory);
                ImGui::MenuItem("Engine Analysis", nullptr, &settings.showEngineAnalysis);
                ImGui::MenuItem("Game Info", nullptr, &settings.showGameInfo);
                ImGui::MenuItem("Evaluation Bar", nullptr, &settings.showEvaluationBar);
                ImGui::Separator();
                if (ImGui::MenuItem("Settings", "F2")) setState(GuiState::SETTINGS);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) showAbout = true;
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void Gui::handleMouseEvents(const sf::Event& event) {
        if (auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                sf::Vector2i mousePosition = mouseButtonPressed->position;

                if (isPositionOnBoard(mousePosition)) {
                    Position boardPos = getBoardPosition(mousePosition);

                    if (boardPos.IsValid()) {
                        const Piece& piece = gameManager.GetBoard().GetPiece(boardPos);

                        if (!piece.IsEmpty() && piece.color == gameManager.GetCurrentPlayer()) {
                            isDragging = true;
                            selectedPosition = boardPos;
                            updatePossibleMoves();

                            sf::Vector2f screenPos = getScreenPosition(boardPos);
                            dragOffset = sf::Vector2f(static_cast<float>(mousePosition.x) - screenPos.x,
                                static_cast<float>(mousePosition.y) - screenPos.y);
                        }
                    }
                }
            }
            if (mouseButtonPressed->button == sf::Mouse::Button::Right && settings.showMoveArrows) {
                sf::Vector2i mousePosition = mouseButtonPressed->position;
                if (isPositionOnBoard(mousePosition)) {
                    Position boardPos = getBoardPosition(mousePosition);
                    if (selectedPosition.IsValid() && boardPos.IsValid()) {
                        addMoveArrow(selectedPosition, boardPos);
                    }
                }
            }
        }
        if (auto* mouseButtonReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseButtonReleased->button == sf::Mouse::Button::Left && isDragging) {
                isDragging = false;
                sf::Vector2i mousePosition = mouseButtonReleased->position;
                if (isPositionOnBoard(mousePosition)) {
                    Position targetPosition = getBoardPosition(mousePosition);
                    if (selectedPosition.IsValid() && targetPosition.IsValid() &&
                        selectedPosition != targetPosition) {
                        makeMove(selectedPosition, targetPosition);
                    }
                }
                selectedPosition = Position(-1, -1);
                possibleMoves.clear();
            }
        }
        if (auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
            mousePos = mouseMoved->position;
        }
    }

    void Gui::handleKeyboardEvents(const sf::Event& event) {
        if (auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
            switch (keyPressed->code) {
            case sf::Keyboard::Key::Escape:
                if (currentState == GuiState::PLAYING) {
                    setState(GuiState::MAIN_MENU);
                }
                break;
            case sf::Keyboard::Key::Space:
                if (currentState == GuiState::PLAYING) {
                    if (isPaused) {
                        resumeGame();
                    }
                    else {
                        pauseGame();
                    }
                }
                break;
            case sf::Keyboard::Key::Z:
                if (keyPressed->control) {
                    undoMove();
                }
                break;
            case sf::Keyboard::Key::Y:
                if (keyPressed->control) {
                    redoMove();
                }
                break;
            case sf::Keyboard::Key::N:
                if (keyPressed->control) {
                    resetGame();
                }
                break;
            case sf::Keyboard::Key::F2:
                setState(GuiState::SETTINGS);
                break;
            case sf::Keyboard::Key::F5:
                clearMoveArrows();
                break;
            default:
                break;
            }
        }
    }

    void Gui::makeMove(const Position& from, const Position& to, PieceType promotion) {
        Move move(from, to);
        if (gameManager.IsLegalMove(move)) {
            gameManager.MakeMove(move);
            lastMoveFrom = from;
            lastMoveTo = to;
            if (settings.enableAnimations) {
                const Piece& piece = gameManager.GetBoard().GetPiece(to);
                startPieceAnimation(from, to, piece.type, piece.color);
            }
            clearMoveArrows();
        }
    }

    void Gui::undoMove() {
        gameManager.UndoLastMove();
    }

    void Gui::redoMove() {
        if (!redoStack.empty()) {
            Move move = redoStack.top();
            redoStack.pop();
            gameManager.MakeMove(move);
            lastMoveFrom = move.from;
            lastMoveTo = move.to;
        }
    }

    void Gui::resetGame() {
        gameManager.Reset();
        gameManager.StartGame();
        lastMoveFrom = Position(-1, -1);
        lastMoveTo = Position(-1, -1);
        selectedPosition = Position(-1, -1);
        possibleMoves.clear();
        clearMoveArrows();
        setState(GuiState::PLAYING);
    }

    void Gui::pauseGame() {
        isPaused = true;
        setState(GuiState::PAUSED);
    }

    void Gui::resumeGame() {
        isPaused = false;
        setState(GuiState::PLAYING);
    }

    void Gui::enterAnalysisMode() {
        isAnalysisMode = true;
    }

    void Gui::exitAnalysisMode() {
        isAnalysisMode = false;
    }

    Position Gui::getBoardPosition(const sf::Vector2i& mousePos) const {
        int x = static_cast<int>((mousePos.x - boardOffset.x) / squareSize);
        int y = static_cast<int>((mousePos.y - boardOffset.y) / squareSize);
        return Position(x, y);
    }

    sf::Vector2f Gui::getScreenPosition(const Position& boardPos) const {
        return sf::Vector2f(boardOffset.x + boardPos.x * squareSize,
            boardOffset.y + boardPos.y * squareSize);
    }

    bool Gui::isPositionOnBoard(const sf::Vector2i& mousePos) const {
        return mousePos.x >= boardOffset.x && mousePos.x < boardOffset.x + boardSize &&
            mousePos.y >= boardOffset.y && mousePos.y < boardOffset.y + boardSize;
    }

    void Gui::updatePossibleMoves() {
        possibleMoves.clear();
        if (!selectedPosition.IsValid()) return;
        auto allMoves = gameManager.GetLegalMoves();
        for (const auto& move : allMoves) {
            if (move.from == selectedPosition) {
                possibleMoves.push_back(move.to);
            }
        }
    }

    void Gui::addMoveArrow(const Position& from, const Position& to, const sf::Color& color) {
        MoveArrow arrow;
        arrow.from = from;
        arrow.to = to;
        arrow.color = color;
        arrow.thickness = 3.0f;
        moveArrows.push_back(arrow);
    }

    void Gui::clearMoveArrows() {
        moveArrows.clear();
    }

    void Gui::startPieceAnimation(const Position& from, const Position& to,
        PieceType piece, Color color) {
        currentAnimation.from = from;
        currentAnimation.to = to;
        currentAnimation.startPos = getScreenPosition(from);
        currentAnimation.endPos = getScreenPosition(to);
        currentAnimation.currentPos = currentAnimation.startPos;
        currentAnimation.progress = 0.0f;
        currentAnimation.active = true;
        currentAnimation.pieceType = piece;
        currentAnimation.pieceColor = color;
    }

    void Gui::updateAnimation(float deltaTime) {
        if (!currentAnimation.active) return;
        currentAnimation.progress += deltaTime / currentAnimation.duration;
        if (currentAnimation.progress >= 1.0f) {
            currentAnimation.progress = 1.0f;
            currentAnimation.active = false;
        }
        float t = currentAnimation.progress;
        t = t * t * (3.0f - 2.0f * t);
        currentAnimation.currentPos = sf::Vector2f(
            currentAnimation.startPos.x + t * (currentAnimation.endPos.x - currentAnimation.startPos.x),
            currentAnimation.startPos.y + t * (currentAnimation.endPos.y - currentAnimation.startPos.y)
        );
    }

    void Gui::initializeThemes() {
        themes.resize(6);
        themes[0] = {
            sf::Color(240, 217, 181),
            sf::Color(181, 136, 99),
            sf::Color(255, 255, 0, 100),
            sf::Color(255, 255, 0, 150),
            sf::Color(255, 255, 0, 80),
            sf::Color(255, 0, 0, 120),
            sf::Color(139, 69, 19),
            "Classic"
        };

        themes[1] = {
            sf::Color(118, 150, 86),
            sf::Color(238, 238, 210),
            sf::Color(255, 255, 0, 100),
            sf::Color(255, 255, 0, 150),
            sf::Color(255, 255, 0, 80),
            sf::Color(255, 0, 0, 120),
            sf::Color(34, 34, 34),
            "Dark"
        };

        themes[2] = {
            sf::Color(222, 227, 230),
            sf::Color(140, 162, 173),
            sf::Color(255, 255, 0, 100),
            sf::Color(255, 255, 0, 150),
            sf::Color(255, 255, 0, 80),
            sf::Color(255, 0, 0, 120),
            sf::Color(0, 100, 150),
            "Blue"
        };

        themes[3] = {
            sf::Color(235, 236, 208),
            sf::Color(119, 153, 84),
            sf::Color(255, 255, 0, 100),
            sf::Color(255, 255, 0, 150),
            sf::Color(255, 255, 0, 80),
            sf::Color(255, 0, 0, 120),
            sf::Color(46, 125, 50),
            "Green"
        };

        themes[4] = {
            sf::Color(234, 233, 210),
            sf::Color(156, 98, 137),
            sf::Color(255, 255, 0, 100),
            sf::Color(255, 255, 0, 150),
            sf::Color(255, 255, 0, 80),
            sf::Color(255, 0, 0, 120),
            sf::Color(106, 27, 154),
            "Purple"
        };

        themes[5] = {
            sf::Color(248, 248, 246),
            sf::Color(75, 115, 153),
            sf::Color(255, 255, 0, 100),
            sf::Color(255, 255, 0, 150),
            sf::Color(255, 255, 0, 80),
            sf::Color(255, 0, 0, 120),
            sf::Color(169, 169, 169),
            "Marble"
        };
    }

    void Gui::applyTheme(BoardTheme theme) {
    }

    sf::Color Gui::getSquareColor(int x, int y) const {
        const ThemeColors& theme = themes.at(static_cast<int>(settings.boardTheme));
        return ((x + y) % 2 == 0) ? theme.lightSquare : theme.darkSquare;
    }

    void Gui::updateBoardLayout() {
        boardOffset.x = 50.0f;
        boardOffset.y = 50.0f;
        float maxBoardSize = std::min(static_cast<float>(settings.windowWidth) - 300, static_cast<float>(settings.windowHeight) - 100);
        if (boardSize > maxBoardSize) {
            boardSize = maxBoardSize;
            squareSize = boardSize / BOARD_SIZE;
        }
    }

    void Gui::loadTextures() {
        loadPieceTextures();
    }

    void Gui::loadPieceTextures() {
        createDefaultPieceTextures();
    }

    void Gui::createDefaultPieceTextures() {
        std::vector<sf::Texture> tempTextures(12);

        for (int i = 0; i < 12; ++i) {
            unsigned int size = static_cast<unsigned int>(squareSize);
            sf::Color pieceColor;
            if (i < 6) {
                pieceColor = sf::Color(255, 255, 255, 220);
            }
            else {
                pieceColor = sf::Color(30, 30, 30, 220);
            }

            // Corrected sf::Image constructor call
            sf::Image image({ size, size }, pieceColor);

            if (!tempTextures[i].loadFromImage(image)) {
                std::cerr << "Failed to load texture for piece " << i << std::endl;
            }
        }

        std::vector<PieceType> pieceTypes = {
            PieceType::PAWN, PieceType::ROOK, PieceType::KNIGHT,
            PieceType::BISHOP, PieceType::QUEEN, PieceType::KING
        };

        pieceSprites.clear();
        for (int i = 0; i < 6; ++i) {
            PieceType type = pieceTypes[i];
            pieceSprites[type].emplace(Color::WHITE, sf::Sprite(tempTextures[i]));
            pieceSprites[type].emplace(Color::BLACK, sf::Sprite(tempTextures[i + 6]));
        }
    }

    void Gui::saveSettings() {
        std::ofstream file("settings.cfg");
        if (file.is_open()) {
            file << "boardTheme=" << static_cast<int>(settings.boardTheme) << "\n";
            file << "pieceSet=" << static_cast<int>(settings.pieceSet) << "\n";
            file << "showCoordinates=" << (settings.showCoordinates ? 1 : 0) << "\n";
            file << "showLastMove=" << (settings.showLastMove ? 1 : 0) << "\n";
            file << "showPossibleMoves=" << (settings.showPossibleMoves ? 1 : 0) << "\n";
            file << "enableSounds=" << (settings.enableSounds ? 1 : 0) << "\n";
            file << "showMoveArrows=" << (settings.showMoveArrows ? 1 : 0) << "\n";
            file << "enableAnimations=" << (settings.enableAnimations ? 1 : 0) << "\n";
            file << "pieceScale=" << settings.pieceScale << "\n";
            file << "windowWidth=" << settings.windowWidth << "\n";
            file << "windowHeight=" << settings.windowHeight << "\n";
            file << "engineDepth=" << settings.engineDepth << "\n";
            file << "showEngineAnalysis=" << (settings.showEngineAnalysis ? 1 : 0) << "\n";
            file << "showMoveHistory=" << (settings.showMoveHistory ? 1 : 0) << "\n";
            file << "showGameInfo=" << (settings.showGameInfo ? 1 : 0) << "\n";
            file << "showEvaluationBar=" << (settings.showEvaluationBar ? 1 : 0) << "\n";
            file.close();
        }
    }

    void Gui::loadSettings() {
        std::ifstream file("settings.cfg");
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                size_t pos = line.find('=');
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 1);
                    try {
                        if (key == "boardTheme") settings.boardTheme = static_cast<BoardTheme>(std::stoi(value));
                        else if (key == "pieceSet") settings.pieceSet = static_cast<PieceSet>(std::stoi(value));
                        else if (key == "showCoordinates") settings.showCoordinates = (std::stoi(value) == 1);
                        else if (key == "showLastMove") settings.showLastMove = (std::stoi(value) == 1);
                        else if (key == "showPossibleMoves") settings.showPossibleMoves = (std::stoi(value) == 1);
                        else if (key == "enableSounds") settings.enableSounds = (std::stoi(value) == 1);
                        else if (key == "showMoveArrows") settings.showMoveArrows = (std::stoi(value) == 1);
                        else if (key == "enableAnimations") settings.enableAnimations = (std::stoi(value) == 1);
                        else if (key == "pieceScale") settings.pieceScale = std::stof(value);
                        else if (key == "windowWidth") settings.windowWidth = std::stoi(value);
                        else if (key == "windowHeight") settings.windowHeight = std::stoi(value);
                        else if (key == "engineDepth") settings.engineDepth = std::stoi(value);
                        else if (key == "showEngineAnalysis") settings.showEngineAnalysis = (std::stoi(value) == 1);
                        else if (key == "showMoveHistory") settings.showMoveHistory = (std::stoi(value) == 1);
                        else if (key == "showGameInfo") settings.showGameInfo = (std::stoi(value) == 1);
                        else if (key == "showEvaluationBar") settings.showEvaluationBar = (std::stoi(value) == 1);
                    }
                    catch (const std::exception& e) {
                        std::cerr << "Error parsing settings file: " << e.what() << std::endl;
                    }
                }
            }
            file.close();
        }
    }

    void Gui::applySettings() {
        applyTheme(settings.boardTheme);
        updateBoardLayout();
        window.setSize(sf::Vector2u(settings.windowWidth, settings.windowHeight));
        loadPieceTextures(); // Reload textures to apply new piece set and scale
    }

    std::string Gui::positionToString(const Position& pos) const {
        if (!pos.IsValid()) return "";
        char file = 'a' + pos.x;
        char rank = '8' - pos.y;
        return std::string(1, file) + std::string(1, rank);
    }

    std::string Gui::moveToString(const Move& move) const {
        return positionToString(move.from) + positionToString(move.to);
    }

    sf::Color Gui::blendColors(const sf::Color& a, const sf::Color& b, float factor) const {
        factor = std::max(0.0f, std::min(1.0f, factor));
        return sf::Color(
            static_cast<std::uint8_t>(a.r + factor * (b.r - a.r)),
            static_cast<std::uint8_t>(a.g + factor * (b.g - a.g)),
            static_cast<std::uint8_t>(a.b + factor * (b.b - a.b)),
            static_cast<std::uint8_t>(a.a + factor * (b.a - a.a))
        );
    }

    void Gui::centerWindow() {
        sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
        int x = (desktop.size.x - settings.windowWidth) / 2;
        int y = (desktop.size.y - settings.windowHeight) / 2;
        window.setPosition(sf::Vector2i(x, y));
    }

    void Gui::setState(GuiState state) {
        currentState = state;
        switch (state) {
        case GuiState::MAIN_MENU:
            break;
        case GuiState::PLAYING:
            isPaused = false;
            isAnalysisMode = false;
            break;
        case GuiState::ANALYSIS:
            isAnalysisMode = true;
            break;
        case GuiState::PAUSED:
            isPaused = true;
            break;
        case GuiState::GAME_OVER:
            break;
        case GuiState::GAME_SETUP:
            break;
        case GuiState::SETTINGS:
            break;
        }
    }

    void Gui::updateEngineEvaluation() {
        // TODO: Hook into your engine later
        // For now, just mock some evaluation data
        currentEvaluation.depth = 12;
        currentEvaluation.nodes = 123456;
        currentEvaluation.score = 0.35f;
        currentEvaluation.bestMove = "e2e4";
        currentEvaluation.principalVariation = { "e2e4", "e7e5", "g1f3" };
    }

    void Gui::showNotification(const std::string& message) {
        // Right now, just print to console
        std::cout << "[NOTIFICATION] " << message << std::endl;
    }

} // namespace Chess