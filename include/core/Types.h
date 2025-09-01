#pragma once

#include <string>
#include <vector>
#include <forward_list>
#include <map>
#include <memory>
#include <chrono>
#include <stdexcept>
#include <sstream>

namespace Chess {

    // Board constants
    constexpr int BOARD_SIZE = 8;

    // Enums (no changes needed here)
    enum class PieceType : int8_t { EMPTY = 0, PAWN = 1, ROOK = 2, KNIGHT = 3, BISHOP = 4, QUEEN = 5, KING = 6 };
    enum class Color : int8_t { WHITE = 1, BLACK = -1, NONE = 0 };
    enum class MoveType : uint8_t { NORMAL = 0, CASTLING = 1, EN_PASSANT = 2, PROMOTION = 3 };
    enum class GameResult : uint8_t { ONGOING = 0, CHECKMATE_WHITE = 1, CHECKMATE_BLACK = 2, STALEMATE = 3, DRAW_50_MOVES = 4, DRAW_REPETITION = 5, DRAW_MATERIAL = 6, RESIGNATION = 7, TIMEOUT_WHITE = 8, TIMEOUT_BLACK = 9 };
    enum class Difficulty : uint8_t { BEGINNER = 0, EASY = 1, MEDIUM = 2, HARD = 3, EXPERT = 4, MASTER = 5, GRANDMASTER = 6 };
    enum class GameMode : uint8_t { HUMAN_VS_HUMAN = 0, HUMAN_VS_AI = 1, AI_VS_AI = 2, FEN_POSITION_SETUP = 3, PUZZLE_MODE = 4 };

    // Structs (Core Fixes are here)
    struct Position {
        int8_t x, y;
        Position() : x(-1), y(-1) {}
        Position(int8_t x_val, int8_t y_val) : x(x_val), y(y_val) {}
        explicit Position(const std::string& algebraic);
        bool IsValid() const { return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE; }
        std::string ToAlgebraic() const;
        bool operator==(const Position& other) const { return x == other.x && y == other.y; }
        bool operator!=(const Position& other) const { return !(*this == other); }
    };

    // In Types.h

    struct Piece {
        PieceType type;
        Color color;

        Piece() : type(PieceType::EMPTY), color(Color::NONE) {}
        Piece(PieceType t, Color c) : type(t), color(c) {}

        bool IsEmpty() const { return type == PieceType::EMPTY; }
        char ToChar() const;

        // --- ADD THESE 3 MISSING DECLARATIONS ---
        int8_t ToLegacyValue() const;
        static Piece FromLegacyValue(int8_t value);
        std::string ToString() const;
        // -----------------------------------------
    };

    // In Types.h

    struct Move {
        Position from;
        Position to;
        MoveType type;
        PieceType promotionPiece;
        Piece capturedPiece;

        Move() : type(MoveType::NORMAL), promotionPiece(PieceType::EMPTY) {}
        Move(Position p_from, Position p_to, MoveType t = MoveType::NORMAL)
            : from(p_from), to(p_to), type(t), promotionPiece(PieceType::EMPTY) {
        }

        bool IsValid() const { return from.IsValid() && to.IsValid(); }

        // --- ADD THESE 2 MISSING DECLARATIONS ---
        std::string ToAlgebraic() const;
        std::string ToUCI() const;
        // -----------------------------------------
    };
    struct TimeControl {
        std::string name;
        std::chrono::milliseconds baseTime;
        std::chrono::milliseconds increment;
        std::chrono::milliseconds remainingTime;
        TimeControl() : baseTime(0), increment(0), remainingTime(0) {}
    };

    // FIXED: Constructor now initializes the 'timeControl' member
    struct PlayerConfig {
        std::string name;
        bool isHuman;
        Difficulty difficulty;
        TimeControl timeControl;
        PlayerConfig(const std::string& n = "Player", bool human = true, Difficulty diff = Difficulty::MEDIUM)
            : name(n), isHuman(human), difficulty(diff), timeControl() {
        }
    };

    // FIXED: Constructor now initializes ALL members to prevent memory corruption
    struct GameConfig {
        GameMode mode;
        PlayerConfig whitePlayer;
        PlayerConfig blackPlayer;
        bool useGui;
        std::string initialFen;
        GameConfig(GameMode gameMode = GameMode::HUMAN_VS_HUMAN)
            : mode(gameMode),
            whitePlayer("White", true, Difficulty::MEDIUM),
            blackPlayer("Black", true, Difficulty::MEDIUM),
            useGui(true),
            initialFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {
        }
    };

    struct GameStats {
        int totalMoves = 0;
        int captures = 0;
        int checks = 0;
        int castles = 0;
    };

    struct MoveHistoryEntry {
        Move move;
        std::string algebraicNotation;
        std::chrono::milliseconds timeSpent;
        float evaluation;
        int fullMoveNumber;
        MoveHistoryEntry(const Move& m, const std::string& notation, std::chrono::milliseconds time, float eval, int moveNum)
            : move(m), algebraicNotation(notation), timeSpent(time), evaluation(eval), fullMoveNumber(moveNum) {
        }
    };

} // namespace Chess