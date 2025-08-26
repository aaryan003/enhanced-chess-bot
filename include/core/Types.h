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

// Chess piece types - using scoped enum for better type safety
enum class PieceType : int8_t {
    EMPTY = 0,
    PAWN = 1,
    ROOK = 2,
    KNIGHT = 3,
    BISHOP = 4,
    QUEEN = 5,
    KING = 6
};

// Player colors
enum class Color : int8_t {
    WHITE = 1,
    BLACK = -1,
    NONE = 0
};

// Move types for special moves
enum class MoveType : uint8_t {
    NORMAL = 0,
    CASTLING = 1,
    EN_PASSANT = 2,
    PROMOTION = 3
};

// Game end conditions
enum class GameResult : uint8_t {
    ONGOING = 0,
    CHECKMATE_WHITE = 1,
    CHECKMATE_BLACK = 2,
    STALEMATE = 3,
    DRAW_50_MOVES = 4,
    DRAW_REPETITION = 5,
    DRAW_MATERIAL = 6,
    RESIGNATION = 7,
    TIMEOUT_WHITE = 8,
    TIMEOUT_BLACK = 9
};

// Difficulty levels for AI
enum class Difficulty : uint8_t {
    BEGINNER = 0,
    EASY = 1,
    MEDIUM = 2,
    HARD = 3,
    EXPERT = 4,
    MASTER = 5,
    GRANDMASTER = 6
};

// Game modes
enum class GameMode : uint8_t {
    HUMAN_VS_HUMAN = 0,
    HUMAN_VS_AI = 1,
    AI_VS_AI = 2,
    FEN_POSITION_SETUP = 3,
    PUZZLE_MODE = 4
};

// Position on the chess board
struct Position {
    int8_t x, y;

    Position() : x(-1), y(-1) {}
    Position(int8_t x, int8_t y) : x(x), y(y) {}
    Position(const std::string& algebraic);

    bool IsValid() const { return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE; }
    std::string ToAlgebraic() const;

    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Position& other) const { return !(*this == other); }
};

// Chess piece with color and type
struct Piece {
    PieceType type;
    Color color;

    Piece() : type(PieceType::EMPTY), color(Color::NONE) {}
    Piece(PieceType t, Color c) : type(t), color(c) {}

    bool IsEmpty() const { return type == PieceType::EMPTY; }
    bool IsWhite() const { return color == Color::WHITE; }
    bool IsBlack() const { return color == Color::BLACK; }

    int8_t ToLegacyValue() const;
    static Piece FromLegacyValue(int8_t value);

    std::string ToString() const;
    char ToChar() const;
};

// Chess move representation
struct Move {
    Position from;
    Position to;
    MoveType type;
    PieceType promotionPiece;
    Piece capturedPiece;

    Move() : type(MoveType::NORMAL), promotionPiece(PieceType::EMPTY) {}
    Move(Position from, Position to, MoveType type = MoveType::NORMAL)
        : from(from), to(to), type(type), promotionPiece(PieceType::EMPTY) {}

    std::string ToAlgebraic() const;
    std::string ToUCI() const;

    bool IsValid() const { return from.IsValid() && to.IsValid(); }
};

// Time control settings
struct TimeControl {
    std::string name;
    std::chrono::milliseconds baseTime;
    std::chrono::milliseconds increment;
    std::chrono::milliseconds remainingTime;

    TimeControl()
        : name("3+0 Blitz"),
          baseTime(3 * 60 * 1000),
          increment(0),
          remainingTime(3 * 60 * 1000) {}

    TimeControl(std::string n, std::chrono::milliseconds base, std::chrono::milliseconds inc = std::chrono::milliseconds{0})
        : name(n), baseTime(base), increment(inc), remainingTime(base) {}
};

// Player configuration
struct PlayerConfig {
    std::string name;
    bool isHuman;
    Difficulty difficulty;
    TimeControl timeControl;

    PlayerConfig(const std::string& n = "Player", bool human = true,
                 Difficulty diff = Difficulty::MEDIUM)
        : name(n), isHuman(human), difficulty(diff) {}
};

// Game configuration
struct GameConfig {
    GameMode mode;
    PlayerConfig whitePlayer;
    PlayerConfig blackPlayer;
    bool useGui;
    std::string initialFen;

    GameConfig() : mode(GameMode::HUMAN_VS_HUMAN), useGui(false), initialFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {}
};

// Game statistics
struct GameStats {
    int totalMoves = 0;
    int captures = 0;
    int checks = 0;
    int castles = 0;
    std::chrono::milliseconds totalTime{0};
    std::chrono::steady_clock::time_point gameStartTime;
};

// Move history entry
struct MoveHistoryEntry {
    Move move;
    std::string algebraicNotation;
    std::chrono::milliseconds timeSpent;
    float evaluation;
    int fullMoveNumber;

    MoveHistoryEntry(const Move& m, const std::string& notation = "",
                     std::chrono::milliseconds time = std::chrono::milliseconds{0},
                     float eval = 0.0f, int moveNum = 0)
        : move(m), algebraicNotation(notation), timeSpent(time), evaluation(eval), fullMoveNumber(moveNum) {}
};

} // namespace Chess
