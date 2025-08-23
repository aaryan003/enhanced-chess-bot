#pragma once

#include "core/Board.h"
#include <array>
#include <random>

namespace Chess {

    // Zobrist hash class to generate unique keys for board positions.
    class ZobristHash {
    public:
        ZobristHash();
        void generate();
        uint64_t getHash(const Board& board);

    private:
        static const int NUM_PIECE_TYPES = 7;
        static const int NUM_COLORS = 2;
        static const int NUM_SQUARES = 64;

        std::array<std::array<std::array<uint64_t, NUM_SQUARES>, NUM_PIECE_TYPES>, NUM_COLORS> pieceKeys;
        uint64_t sideToMoveKey;
        std::array<uint64_t, 16> castleKeys;
        std::array<uint64_t, 8> enPassantKeys;

        uint64_t randomUInt64();
    };

} // namespace Chess