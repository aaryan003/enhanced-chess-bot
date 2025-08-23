#include "engine/Engine.h"
#include "engine/ZobristHash.h"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <random>

namespace Chess {

// ZobristHash implementation
uint64_t ZobristHash::randomUInt64() {
    std::mt19937_64 rng(std::random_device{}());
    return rng();
}

ZobristHash::ZobristHash() {
    generate();
}

void ZobristHash::generate() {
    std::mt19937_64 rng(std::random_device{}());
    for (int c = 0; c < NUM_COLORS; ++c) {
        for (int p = 0; p < NUM_PIECE_TYPES; ++p) {
            for (int s = 0; s < NUM_SQUARES; ++s) {
                pieceKeys[c][p][s] = rng();
            }
        }
    }
    sideToMoveKey = rng();
    for (int i = 0; i < 16; ++i) {
        castleKeys[i] = rng();
    }
    for (int i = 0; i < 8; ++i) {
        enPassantKeys[i] = rng();
    }
}

uint64_t ZobristHash::getHash(const Board& board) {
    uint64_t hash = 0;
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            const Piece& piece = board.GetPiece(x, y);
            if (!piece.IsEmpty()) {
                int colorIdx = (piece.color == Color::WHITE) ? 0 : 1;
                int pieceIdx = static_cast<int>(piece.type);
                hash ^= pieceKeys[colorIdx][pieceIdx][y * 8 + x];
            }
        }
    }
    if (board.GetCurrentPlayer() == Color::WHITE) {
        hash ^= sideToMoveKey;
    }
    uint8_t castleRights = 0;
    if (board.CanCastleKingSide(Color::WHITE)) castleRights |= 1;
    if (board.CanCastleQueenSide(Color::WHITE)) castleRights |= 2;
    if (board.CanCastleKingSide(Color::BLACK)) castleRights |= 4;
    if (board.CanCastleQueenSide(Color::BLACK)) castleRights |= 8;
    hash ^= castleKeys[castleRights];
    Position enPassantPos = board.GetEnPassantTarget();
    if (enPassantPos.IsValid()) {
        hash ^= enPassantKeys[enPassantPos.x];
    }
    return hash;
}

// Engine implementation
Engine::Engine() {
    for (int i = 0; i < BOARD_SIZE; ++i) {
        for (int j = 0; j < BOARD_SIZE; ++j) {
            historyHeuristic[i][j] = 0;
        }
    }
}

/**
 * The main entry point for the AI to find a move.
 * It uses iterative deepening to search the board.
 */
Move Engine::findBestMove(Board board, Difficulty difficulty, const TimeControl& timeControl) {
    startTime = std::chrono::steady_clock::now();

    // Use adaptive time allocation
    auto totalTime = timeControl.baseTime + timeControl.increment * (board.GetFullMoveNumber() - 1);
    timeLimit = totalTime / 30; // Allocate a portion of total time for the move
    if (timeLimit < std::chrono::milliseconds(100)) timeLimit = std::chrono::milliseconds(100);

    Move bestMove;
    int bestScore = std::numeric_limits<int>::min();

    // We will search up to depth 6, or more if time permits.
    for (int depth = 1; depth <= 12; ++depth) {
        std::vector<Move> legalMoves = board.GetAllLegalMoves(board.GetCurrentPlayer());
        auto orderedMoves = orderMoves(board, legalMoves);

        int currentBestScore = std::numeric_limits<int>::min();
        Move currentBestMove;

        for (const auto& move : orderedMoves) {
            if (timeIsUp()) {
                goto end_search;
            }
            Board tempBoard = board;
            if (tempBoard.MakeMove(move)) {
                int score = -alphaBeta(tempBoard, depth - 1, -std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
                if (score > currentBestScore) {
                    currentBestScore = score;
                    currentBestMove = move;
                }
            }
        }
        bestScore = currentBestScore;
        bestMove = currentBestMove;
    }

end_search:
    return bestMove;
}

/**
 * Implements the Minimax algorithm with Alpha-Beta pruning.
 */
int Engine::alphaBeta(Board& board, int depth, int alpha, int beta) {
    uint64_t hash = zobrist.getHash(board);
    if (transpositionTable.count(hash)) {
        const auto& entry = transpositionTable.at(hash);
        if (entry.depth >= depth) {
            if (entry.bound == TranspositionEntry::BoundType::EXACT) {
                return entry.score;
            }
            if (entry.bound == TranspositionEntry::BoundType::LOWERBOUND) {
                alpha = std::max(alpha, entry.score);
            }
            if (entry.bound == TranspositionEntry::BoundType::UPPERBOUND) {
                beta = std::min(beta, entry.score);
            }
            if (alpha >= beta) {
                return entry.score;
            }
        }
    }

    if (depth == 0 || board.GetGameResult() != GameResult::ONGOING) {
        return quiescenceSearch(board, alpha, beta);
    }

    std::vector<Move> legalMoves = board.GetAllLegalMoves(board.GetCurrentPlayer());
    if (legalMoves.empty()) {
        return static_cast<int>(board.EvaluatePosition(board.GetCurrentPlayer()));
    }

    int score;
    Move bestMoveThisDepth;

    if (board.GetCurrentPlayer() == Color::WHITE) {
        score = std::numeric_limits<int>::min();
        auto orderedMoves = orderMoves(board, legalMoves);
        for (const auto& move : orderedMoves) {
            Board tempBoard = board;
            if (tempBoard.MakeMove(move)) {
                int eval = alphaBeta(tempBoard, depth - 1, alpha, beta);
                if (eval > score) {
                    score = eval;
                    bestMoveThisDepth = move;
                }
                alpha = std::max(alpha, eval);
                if (beta <= alpha) {
                    break;
                }
            }
        }
    } else {
        score = std::numeric_limits<int>::max();
        auto orderedMoves = orderMoves(board, legalMoves);
        for (const auto& move : orderedMoves) {
            Board tempBoard = board;
            if (tempBoard.MakeMove(move)) {
                int eval = alphaBeta(tempBoard, depth - 1, alpha, beta);
                if (eval < score) {
                    score = eval;
                    bestMoveThisDepth = move;
                }
                beta = std::min(beta, eval);
                if (beta <= alpha) {
                    break;
                }
            }
        }
    }

    TranspositionEntry::BoundType bound;
    if (score <= alpha) {
        bound = TranspositionEntry::BoundType::UPPERBOUND;
    } else if (score >= beta) {
        bound = TranspositionEntry::BoundType::LOWERBOUND;
    } else {
        bound = TranspositionEntry::BoundType::EXACT;
    }

    transpositionTable[zobrist.getHash(board)] = {score, depth, bestMoveThisDepth, bound};

    return score;
}

/**
 * Quiescence search to handle tactical positions.
 */
int Engine::quiescenceSearch(Board& board, int alpha, int beta) {
    if (timeIsUp()) {
        return 0;
    }

    int standPat = static_cast<int>(board.EvaluatePosition(board.GetCurrentPlayer()));

    if (standPat >= beta) {
        return beta;
    }
    if (standPat > alpha) {
        alpha = standPat;
    }

    auto legalMoves = board.GetAllLegalMoves(board.GetCurrentPlayer());
    for (const auto& move : legalMoves) {
        if (move.capturedPiece.IsEmpty() && !board.IsInCheck(board.GetCurrentPlayer())) {
            continue;
        }
        Board tempBoard = board;
        if (tempBoard.MakeMove(move)) {
            int score = -quiescenceSearch(tempBoard, -beta, -alpha);
            if (score >= beta) {
                return beta;
            }
            if (score > alpha) {
                alpha = score;
            }
        }
    }
    return alpha;
}

/**
 * Move ordering function.
 */
std::vector<Move> Engine::orderMoves(const Board& board, const std::vector<Move>& moves) {
    std::vector<std::pair<int, Move>> scoredMoves;
    for (const auto& move : moves) {
        int score = 0;
        if (!move.capturedPiece.IsEmpty()) {
            score += 10 * static_cast<int>(board.GetPieceValue(move.capturedPiece.type)) - static_cast<int>(board.GetPieceValue(board.GetPiece(move.from).type));
        }
        score += historyHeuristic[move.from.y][move.from.x];
        scoredMoves.push_back({score, move});
    }
    std::sort(scoredMoves.begin(), scoredMoves.end(), [](const auto& a, const auto& b) {
        return a.first > b.first;
    });
    std::vector<Move> orderedMoves;
    for (const auto& pair : scoredMoves) {
        orderedMoves.push_back(pair.second);
    }
    return orderedMoves;
}

/**
 * Checks if the time limit for the search has been exceeded.
 */
bool Engine::timeIsUp() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime) >= timeLimit;
}

} // namespace Chess
