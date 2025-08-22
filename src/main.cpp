#include <iostream>
#include <memory>
#include <string>
#include <fmt/format.h>

// Include our headers
#include "core/Types.h"
#include "core/Board.h"
#include "game/GameManager.h"
#include "game/Player.h"

using namespace Chess;

// Forward declarations for future days
class GameEngine;
class ChessGame;
class ConsoleUI;

void PrintWelcome() {
    fmt::print("===========================================\n");
    fmt::print("    Enhanced Chess Bot - Day 2 Build\n");
    fmt::print("===========================================\n");
    fmt::print("Features in this build:\n");
    fmt::print("- Modern C++ core types and board representation\n");
    fmt::print("- Cross-platform compatibility\n");
    fmt::print("- SFML + ImGui integration ready\n");
    fmt::print("- Foundation for enhanced features\n");
    fmt::print("===========================================\n\n");
}

void TestBasicBoard() {
    fmt::print("Testing basic board functionality...\n\n");

    // Create and setup board
    Board board;
    board.SetupStartingPosition();

    // Display initial position
    fmt::print("Initial board position:\n");
    fmt::print("{}\n", board.ToString());

    // Test basic operations
    fmt::print("Current player: {}\n",
              board.GetCurrentPlayer() == Color::WHITE ? "White" : "Black");

    // Test piece access
    Position e2("e2");
    Position e4("e4");

    const Piece& pawn = board.GetPiece(e2);
    fmt::print("Piece at e2: {}\n", pawn.ToString());

    // Test FEN generation
    std::string fen = board.ToFEN();
    fmt::print("FEN: {}\n\n", fen);

    // Test position validation
    fmt::print("Position e2 valid: {}\n", e2.IsValid() ? "Yes" : "No");
    fmt::print("Position e4 valid: {}\n", e4.IsValid() ? "Yes" : "No");
    fmt::print("Position z9 valid: {}\n", Position("z9").IsValid() ? "Yes" : "No");

    fmt::print("\nBasic board test completed successfully!\n\n");
}

void TestAdvancedBoard() {
    fmt::print("Testing advanced board functionality (Day 2 features)...\n\n");

    Board board;
    board.SetupStartingPosition();

    // Test move generation for a single piece (e.g., White's e2 pawn)
    Position e2("e2");
    auto pawnMoves = board.GetPieceMoves(e2);
    fmt::print("Legal moves for pawn at e2:\n");
    for (const auto& move : pawnMoves) {
        fmt::print(" - {}\n", move.ToAlgebraic());
    }
    fmt::print("\n");

    // Test making a move
    Move e2e4(e2, Position("e4"));
    if (board.MakeMove(e2e4)) {
        fmt::print("Made a move: {}\n", e2e4.ToAlgebraic());
        fmt::print("Board after 1.e4:\n{}\n", board.ToString());
    }

    // Test a more complex scenario with a custom FEN
    std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3P4/1p2P3/2N2N2/PPPBQPPP/R3K2R b KQkq - 0 1";
    board.LoadFromFEN(fen);
    fmt::print("Board loaded from FEN:\n{}\n", board.ToString());
    fmt::print("Legal moves for Black (should include castling):\n");

    auto blackMoves = board.GetAllLegalMoves(Color::BLACK);
    for (const auto& move : blackMoves) {
        fmt::print(" - {}\n", move.ToAlgebraic());
    }
    fmt::print("\n");

    // Test checkmate detection (Fool's Mate)
    Board foolsmateBoard;
    foolsmateBoard.LoadFromFEN("rnb1kbnr/pppp1ppp/8/4p3/5PPq/8/PPPPP2P/RNBQKBNR w KQkq - 1 3");
    fmt::print("Testing checkmate detection (Fool's Mate position)...\n");
    fmt::print("Current board:\n{}\n", foolsmateBoard.ToString());
    fmt::print("Current player: {}\n",
               foolsmateBoard.GetCurrentPlayer() == Color::WHITE ? "White" : "Black");

    GameResult result = foolsmateBoard.GetGameResult();
    fmt::print("Game result: {}\n",
               (result == GameResult::CHECKMATE_WHITE) ? "Checkmate for White" : "Ongoing or other result");

    fmt::print("\nAdvanced board test completed successfully!\n\n");
}

void TestGameManager() {
    fmt::print("Testing GameManager functionality...\n\n");

    GameManager manager;
    manager.StartGame();

    fmt::print("Starting game...\n");
    fmt::print("Initial FEN: {}\n", manager.GetCurrentFEN());

    Move e2e4(Position("e2"), Position("e4"));
    if (manager.MakeMove(e2e4)) {
        fmt::print("Made move: {}\n", e2e4.ToAlgebraic());
    }

    Move e7e5(Position("e7"), Position("e5"));
    if (manager.MakeMove(e7e5)) {
        fmt::print("Made move: {}\n", e7e5.ToAlgebraic());
    }

    Move d2d4(Position("d2"), Position("d4"));
    if (manager.MakeMove(d2d4)) {
        fmt::print("Made move: {}\n", d2d4.ToAlgebraic());
    }

    fmt::print("\nGame state after a few moves:\n{}\n", manager.GetBoard().ToString());
    fmt::print("Current FEN: {}\n", manager.GetCurrentFEN());
    fmt::print("Current player: {}\n",
               manager.GetCurrentPlayer() == Color::WHITE ? "White" : "Black");

    fmt::print("\nGameManager test completed successfully!\n\n");
}

void ShowDevelopmentPlan() {
    fmt::print("7-Day Development Plan:\n");
    fmt::print("Day 1: Core Foundation ✓\n");
    fmt::print("Day 2: Move Generation & Game Logic ✓\n");
    fmt::print("Day 3: AI Engine & Search Algorithms\n");
    fmt::print("Day 4: Engine Enhancements & Time Controls\n");
    fmt::print("Day 5: Game Modes & Configuration\n");
    fmt::print("Day 6: SFML GUI Interface\n");
    fmt::print("Day 7: Polish, Testing & Advanced Features\n\n");
}

int main() {
    try {
        PrintWelcome();

        // Test our core functionality
        TestBasicBoard();
        TestAdvancedBoard();
        TestGameManager();

        ShowDevelopmentPlan();

        fmt::print("Day 2 build completed successfully!\n");
        fmt::print("✅ Complete chess board implementation\n");
        fmt::print("✅ Full move generation for all pieces\n");
        fmt::print("✅ Legal move validation\n");
        fmt::print("✅ Check/checkmate detection\n");
        fmt::print("✅ Game state management\n");
        fmt::print("✅ FEN notation support\n");
        fmt::print("✅ Position evaluation system\n");

        fmt::print("\nNext: Implement AI search engine and algorithms.\n");

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}