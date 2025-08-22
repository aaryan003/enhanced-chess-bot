#pragma once

#include "core/Types.h"
#include <array>
#include <vector>
#include <string>

namespace Chess {

class Board {
private:
    std::array<std::array<Piece, BOARD_SIZE>, BOARD_SIZE> squares;
    Color currentPlayer;

    // Castling rights
    bool whiteKingSideCastle;
    bool whiteQueenSideCastle;
    bool blackKingSideCastle;
    bool blackQueenSideCastle;

    // En passant
    Position enPassantTarget;

    // Move counters
    int halfMoveClock;  // For 50-move rule
    int fullMoveNumber;

    // Game history for repetition detection
    std::vector<std::string> positionHistory;

public:
    Board();
    Board(const Board& other) = default;
    Board& operator=(const Board& other) = default;

    // Board setup
    void SetupStartingPosition();
    void Clear();

    // Piece access
    const Piece& GetPiece(const Position& pos) const;
    const Piece& GetPiece(int x, int y) const;
    void SetPiece(const Position& pos, const Piece& piece);
    void SetPiece(int x, int y, const Piece& piece);
    bool IsEmpty(const Position& pos) const;

    // Game state
    Color GetCurrentPlayer() const { return currentPlayer; }
    void SetCurrentPlayer(Color color) { currentPlayer = color; }
    void SwitchPlayer() { currentPlayer = (currentPlayer == Color::WHITE) ? Color::BLACK : Color::WHITE; }

    // Castling rights
    bool CanCastleKingSide(Color color) const;
    bool CanCastleQueenSide(Color color) const;
    void SetCastlingRights(Color color, bool kingSide, bool queenSide);
    void DisableCastling(Color color, bool kingSide, bool queenSide);

    // En passant
    Position GetEnPassantTarget() const { return enPassantTarget; }
    void SetEnPassantTarget(const Position& target) { enPassantTarget = target; }
    void ClearEnPassantTarget() { enPassantTarget = Position(); }

    // Move counters
    int GetHalfMoveClock() const { return halfMoveClock; }
    int GetFullMoveNumber() const { return fullMoveNumber; }
    void IncrementHalfMoveClock() { halfMoveClock++; }
    void ResetHalfMoveClock() { halfMoveClock = 0; }
    void IncrementFullMoveNumber() { fullMoveNumber++; }

    // Position analysis
    Position FindKing(Color color) const;
    bool IsInCheck(Color color) const;
    bool IsSquareAttacked(const Position& pos, Color attackingColor) const;
    std::vector<Move> GetAllLegalMoves(Color color) const;
    std::vector<Move> GetPieceMoves(const Position& pos) const;

    // Move validation and execution
    bool IsLegalMove(const Move& move) const;
    bool MakeMove(const Move& move);
    bool UndoMove(const Move& move);  // For search algorithms

    // Game state evaluation
    GameResult GetGameResult() const;
    bool IsStalemate(Color color) const;
    bool IsCheckmate(Color color) const;
    bool IsInsufficientMaterial() const;
    bool IsThreefoldRepetition() const;

    // Board evaluation (for AI)
    float EvaluatePosition(Color perspective) const;
    float GetPieceValue(PieceType type) const;
    float GetPositionValue(const Position& pos, PieceType type, Color color) const;

    // String representations
    std::string ToFEN() const;
    bool LoadFromFEN(const std::string& fen);
    std::string ToString() const;  // Human-readable format

    // Position hashing for repetition detection
    std::string GetPositionHash() const;
    void AddToHistory();
    void ClearHistory();

    // Utility functions
    bool IsValidPosition(const Position& pos) const;
    bool IsValidPosition(int x, int y) const;
    static bool IsValidSquare(int x, int y);

private:
    // Internal move generation helpers
    std::vector<Move> GetPawnMoves(const Position& pos) const;
    std::vector<Move> GetRookMoves(const Position& pos) const;
    std::vector<Move> GetKnightMoves(const Position& pos) const;
    std::vector<Move> GetBishopMoves(const Position& pos) const;
    std::vector<Move> GetQueenMoves(const Position& pos) const;
    std::vector<Move> GetKingMoves(const Position& pos) const;

    // Helper functions for move validation
    std::vector<Move> GetSlidingMoves(const Position& pos, const std::vector<Position>& directions) const;
    bool WouldBeInCheck(const Move& move, Color color) const;
    void UpdateCastlingRights(const Move& move);
    void UpdateEnPassant(const Move& move);

    // Position evaluation helpers
    static const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> PAWN_TABLE;
    static const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> KNIGHT_TABLE;
    static const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> BISHOP_TABLE;
    static const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> ROOK_TABLE;
    static const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> QUEEN_TABLE;
    static const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> KING_TABLE;
};

} // namespace Chess