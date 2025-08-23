#pragma once

#include "core/Board.h"
#include "core/Types.h"
#include "engine/ZobristHash.h" // Include the full definition
#include <chrono>
#include <memory>
#include <limits>
#include <vector>
#include <unordered_map>

namespace Chess {

/**
 * @class Engine
 * @brief Implements the core chess AI logic with advanced search techniques.
 *
 * This class is a major refactor of the Day 3 engine, adding iterative deepening,
 * transposition tables, and move ordering to improve performance and strength.
 */
class Engine {
public:
    // A simplified representation of a Transposition Table Entry.
    struct TranspositionEntry {
        // The value of a score relative to the alpha-beta bounds
        enum class BoundType { EXACT, LOWERBOUND, UPPERBOUND };

        int score;
        int depth;
        Move bestMove;
        BoundType bound;
    };

private:
    std::chrono::milliseconds timeLimit;
    std::chrono::steady_clock::time_point startTime;

    // The transposition table for position caching.
    std::unordered_map<uint64_t, TranspositionEntry> transpositionTable;

    // The Zobrist hash key for the current board state.
    ZobristHash zobrist;

    // Arrays for move ordering heuristics.
    // We'll use these to prioritize promising moves.
    std::array<std::array<int, BOARD_SIZE>, BOARD_SIZE> historyHeuristic;
    std::array<std::array<Move, BOARD_SIZE>, BOARD_SIZE> killerMoves;

    // The core recursive function for the Minimax search with alpha-beta pruning.
    int alphaBeta(Board& board, int depth, int alpha, int beta);

    // This is the move ordering function that prioritizes promising moves.
    std::vector<Move> orderMoves(const Board& board, const std::vector<Move>& moves);

    // Checks if the time limit for the search has been exceeded.
    bool timeIsUp() const;

    // Quiescence search to handle noisy positions at the end of the search.
    int quiescenceSearch(Board& board, int alpha, int beta);

public:
    /**
     * @brief Constructor for the Engine.
     */
    Engine();

    /**
     * @brief Finds the best move for the current position.
     *
     * @param board The current state of the board.
     * @param difficulty The difficulty level to determine search depth.
     * @param timeControl The time control settings for time management.
     * @return The best move found, or an invalid move if no legal moves exist.
     */
    Move findBestMove(Board board, Difficulty difficulty, const TimeControl& timeControl);
};

} // namespace Chess