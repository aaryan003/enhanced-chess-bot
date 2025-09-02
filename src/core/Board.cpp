#include "core/Board.h"
#include <fmt/format.h>
#include <sstream>
#include <algorithm>
#include <unordered_set>

namespace Chess {

// Piece-square tables for position evaluation (from white's perspective)
const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> Board::PAWN_TABLE = {{
    {{ 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0}},
    {{ 5.0,  5.0,  5.0,  5.0,  5.0,  5.0,  5.0,  5.0}},
    {{ 1.0,  1.0,  2.0,  3.0,  3.0,  2.0,  1.0,  1.0}},
    {{ 0.5,  0.5,  1.0,  2.5,  2.5,  1.0,  0.5,  0.5}},
    {{ 0.0,  0.0,  0.0,  2.0,  2.0,  0.0,  0.0,  0.0}},
    {{ 0.5, -0.5, -1.0,  0.0,  0.0, -1.0, -0.5,  0.5}},
    {{ 0.5,  1.0,  1.0, -2.0, -2.0,  1.0,  1.0,  0.5}},
    {{ 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0}}
}};

const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> Board::KNIGHT_TABLE = {{
    {{-5.0, -4.0, -3.0, -3.0, -3.0, -3.0, -4.0, -5.0}},
    {{-4.0, -2.0,  0.0,  0.0,  0.0,  0.0, -2.0, -4.0}},
    {{-3.0,  0.0,  1.0,  1.5,  1.5,  1.0,  0.0, -3.0}},
    {{-3.0,  0.5,  1.5,  2.0,  2.0,  1.5,  0.5, -3.0}},
    {{-3.0,  0.0,  1.5,  2.0,  2.0,  1.5,  0.0, -3.0}},
    {{-3.0,  0.5,  1.0,  1.5,  1.5,  1.0,  0.5, -3.0}},
    {{-4.0, -2.0,  0.0,  0.5,  0.5,  0.0, -2.0, -4.0}},
    {{-5.0, -4.0, -3.0, -3.0, -3.0, -3.0, -4.0, -5.0}}
}};

const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> Board::BISHOP_TABLE = {{
    {{-2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -2.0}},
    {{-1.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -1.0}},
    {{-1.0,  0.0,  0.5,  1.0,  1.0,  0.5,  0.0, -1.0}},
    {{-1.0,  0.5,  0.5,  1.0,  1.0,  0.5,  0.5, -1.0}},
    {{-1.0,  0.0,  1.0,  1.0,  1.0,  1.0,  0.0, -1.0}},
    {{-1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0, -1.0}},
    {{-1.0,  0.5,  0.0,  0.0,  0.0,  0.0,  0.5, -1.0}},
    {{-2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -2.0}}
}};

const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> Board::ROOK_TABLE = {{
    {{ 0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0}},
    {{ 0.5,  1.0,  1.0,  1.0,  1.0,  1.0,  1.0,  0.5}},
    {{-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5}},
    {{-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5}},
    {{-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5}},
    {{-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5}},
    {{-0.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -0.5}},
    {{ 0.0,  0.0,  0.0,  0.5,  0.5,  0.0,  0.0,  0.0}}
}};

const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> Board::QUEEN_TABLE = {{
    {{-2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0}},
    {{-1.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -1.0}},
    {{-1.0,  0.0,  0.5,  0.5,  0.5,  0.5,  0.0, -1.0}},
    {{-0.5,  0.0,  0.5,  0.5,  0.5,  0.5,  0.0, -0.5}},
    {{ 0.0,  0.0,  0.5,  0.5,  0.5,  0.5,  0.0, -0.5}},
    {{-1.0,  0.5,  0.5,  0.5,  0.5,  0.5,  0.0, -1.0}},
    {{-1.0,  0.0,  0.5,  0.0,  0.0,  0.0,  0.0, -1.0}},
    {{-2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0}}
}};

const std::array<std::array<float, BOARD_SIZE>, BOARD_SIZE> Board::KING_TABLE = {{
    {{-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0}},
    {{-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0}},
    {{-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0}},
    {{-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0}},
    {{-2.0, -3.0, -3.0, -4.0, -4.0, -3.0, -3.0, -2.0}},
    {{-1.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -1.0}},
    {{ 2.0,  2.0,  0.0,  0.0,  0.0,  0.0,  2.0,  2.0}},
    {{ 2.0,  3.0,  1.0,  0.0,  0.0,  1.0,  3.0,  2.0}}
}};

Board::Board()
    : currentPlayer(Color::WHITE),
    whiteKingSideCastle(false),
    whiteQueenSideCastle(false),
    blackKingSideCastle(false),
    blackQueenSideCastle(false),
    enPassantTarget(),
    halfMoveClock(0),
    fullMoveNumber(1)
{
    // Initialize the board array to empty
    Clear();
}

void Board::SetupStartingPosition() {
    Clear();

    // Set up pawns
    for (int x = 0; x < BOARD_SIZE; ++x) {
        SetPiece(x, 1, Piece(PieceType::PAWN, Color::BLACK));
        SetPiece(x, 6, Piece(PieceType::PAWN, Color::WHITE));
    }

    // Set up other pieces
    const std::array<PieceType, 8> backRankOrder = {
        PieceType::ROOK, PieceType::KNIGHT, PieceType::BISHOP, PieceType::QUEEN,
        PieceType::KING, PieceType::BISHOP, PieceType::KNIGHT, PieceType::ROOK
    };

    for (int x = 0; x < BOARD_SIZE; ++x) {
        SetPiece(x, 0, Piece(backRankOrder[x], Color::BLACK));
        SetPiece(x, 7, Piece(backRankOrder[x], Color::WHITE));
    }

    // Set initial game state
    currentPlayer = Color::WHITE;
    whiteKingSideCastle = whiteQueenSideCastle = true;
    blackKingSideCastle = blackQueenSideCastle = true;
    enPassantTarget = Position();
    halfMoveClock = 0;
    fullMoveNumber = 1;

    AddToHistory();
}

void Board::Clear() {
    // Initialize all squares to empty
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            squares[y][x] = Piece();  // Empty piece
        }
    }

    currentPlayer = Color::WHITE;
    whiteKingSideCastle = whiteQueenSideCastle = false;
    blackKingSideCastle = blackQueenSideCastle = false;
    enPassantTarget = Position();
    halfMoveClock = 0;
    fullMoveNumber = 1;
    positionHistory.clear();
}

const Piece& Board::GetPiece(const Position& pos) const {
    return GetPiece(pos.x, pos.y);
}

const Piece& Board::GetPiece(int x, int y) const {
    if (!IsValidPosition(x, y)) {
        static const Piece empty;
        return empty;
    }
    return squares[y][x];
}

void Board::SetPiece(const Position& pos, const Piece& piece) {
    SetPiece(pos.x, pos.y, piece);
}

void Board::SetPiece(int x, int y, const Piece& piece) {
    if (IsValidPosition(x, y)) {
        squares[y][x] = piece;
    }
}

bool Board::IsEmpty(const Position& pos) const {
    return GetPiece(pos).IsEmpty();
}

bool Board::CanCastleKingSide(Color color) const {
    return color == Color::WHITE ? whiteKingSideCastle : blackKingSideCastle;
}

bool Board::CanCastleQueenSide(Color color) const {
    return color == Color::WHITE ? whiteQueenSideCastle : blackQueenSideCastle;
}

void Board::SetCastlingRights(Color color, bool kingSide, bool queenSide) {
    if (color == Color::WHITE) {
        whiteKingSideCastle = kingSide;
        whiteQueenSideCastle = queenSide;
    } else {
        blackKingSideCastle = kingSide;
        blackQueenSideCastle = queenSide;
    }
}

void Board::DisableCastling(Color color, bool kingSide, bool queenSide) {
    if (color == Color::WHITE) {
        if (kingSide) whiteKingSideCastle = false;
        if (queenSide) whiteQueenSideCastle = false;
    } else {
        if (kingSide) blackKingSideCastle = false;
        if (queenSide) blackQueenSideCastle = false;
    }
}

Position Board::FindKing(Color color) const {
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            const Piece& piece = GetPiece(x, y);
            if (piece.type == PieceType::KING && piece.color == color) {
                return Position(x, y);
            }
        }
    }
    return Position(); // Invalid position if king not found
}

bool Board::IsInCheck(Color color) const {
    Position kingPos = FindKing(color);
    if (!kingPos.IsValid()) return false;

    Color enemyColor = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;
    return IsSquareAttacked(kingPos, enemyColor);
}

bool Board::IsSquareAttacked(const Position& pos, Color attackingColor) const {
    // Check for pawn attacks
    int pawnDirection = (attackingColor == Color::WHITE) ? -1 : 1;
    Position pawnLeft(pos.x - 1, pos.y + pawnDirection);
    Position pawnRight(pos.x + 1, pos.y + pawnDirection);

    if (pawnLeft.IsValid() && GetPiece(pawnLeft).type == PieceType::PAWN &&
        GetPiece(pawnLeft).color == attackingColor) {
        return true;
    }
    if (pawnRight.IsValid() && GetPiece(pawnRight).type == PieceType::PAWN &&
        GetPiece(pawnRight).color == attackingColor) {
        return true;
    }

    // Check for knight attacks
    const std::array<Position, 8> knightMoves = {
        Position(-2, -1), Position(-2, 1), Position(-1, -2), Position(-1, 2),
        Position(1, -2), Position(1, 2), Position(2, -1), Position(2, 1)
    };

    for (const auto& offset : knightMoves) {
        Position checkPos(pos.x + offset.x, pos.y + offset.y);
        if (checkPos.IsValid() && GetPiece(checkPos).type == PieceType::KNIGHT &&
            GetPiece(checkPos).color == attackingColor) {
            return true;
        }
    }

    // Check for sliding piece attacks (rook, bishop, queen)
    const std::array<Position, 8> directions = {
        Position(-1, 0), Position(1, 0), Position(0, -1), Position(0, 1),  // Rook directions
        Position(-1, -1), Position(-1, 1), Position(1, -1), Position(1, 1) // Bishop directions
    };

    for (int i = 0; i < 8; ++i) {
        const Position& dir = directions[i];
        Position checkPos(pos.x + dir.x, pos.y + dir.y);

        while (checkPos.IsValid()) {
            const Piece& piece = GetPiece(checkPos);
            if (!piece.IsEmpty()) {
                if (piece.color == attackingColor) {
                    // Check if this piece can attack in this direction
                    if (piece.type == PieceType::QUEEN ||
                        (piece.type == PieceType::ROOK && i < 4) ||
                        (piece.type == PieceType::BISHOP && i >= 4)) {
                        return true;
                    }
                }
                break; // Piece blocks further movement
            }
            checkPos.x += dir.x;
            checkPos.y += dir.y;
        }
    }

    // Check for king attacks (adjacent squares)
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;
            Position checkPos(pos.x + dx, pos.y + dy);
            if (checkPos.IsValid() && GetPiece(checkPos).type == PieceType::KING &&
                GetPiece(checkPos).color == attackingColor) {
                return true;
            }
        }
    }

    return false;
}

std::vector<Move> Board::GetAllLegalMoves(Color color) const {
    std::vector<Move> legalMoves;

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            const Piece& piece = GetPiece(x, y);
            if (piece.color == color) {
                auto pieceMoves = GetPieceMoves(Position(x, y));
                legalMoves.insert(legalMoves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
    }

    return legalMoves;
}

std::vector<Move> Board::GetPieceMoves(const Position& pos) const {
    const Piece& piece = GetPiece(pos);
    if (piece.IsEmpty()) return {};

    switch (piece.type) {
        case PieceType::PAWN: return GetPawnMoves(pos);
        case PieceType::ROOK: return GetRookMoves(pos);
        case PieceType::KNIGHT: return GetKnightMoves(pos);
        case PieceType::BISHOP: return GetBishopMoves(pos);
        case PieceType::QUEEN: return GetQueenMoves(pos);
        case PieceType::KING: return GetKingMoves(pos);
        default: return {};
    }
}

std::vector<Move> Board::GetPawnMoves(const Position& pos) const {
    std::vector<Move> moves;
    const Piece& piece = GetPiece(pos);
    Color color = piece.color;

    int direction = (color == Color::WHITE) ? -1 : 1;
    int startRank = (color == Color::WHITE) ? 6 : 1;
    int promotionRank = (color == Color::WHITE) ? 0 : 7;

    // Forward move
    Position forward(pos.x, pos.y + direction);
    if (forward.IsValid() && IsEmpty(forward)) {
        if (forward.y == promotionRank) {
            // Promotion moves
            moves.emplace_back(pos, forward, MoveType::PROMOTION);
            moves.back().promotionPiece = PieceType::QUEEN;
            moves.emplace_back(pos, forward, MoveType::PROMOTION);
            moves.back().promotionPiece = PieceType::ROOK;
            moves.emplace_back(pos, forward, MoveType::PROMOTION);
            moves.back().promotionPiece = PieceType::BISHOP;
            moves.emplace_back(pos, forward, MoveType::PROMOTION);
            moves.back().promotionPiece = PieceType::KNIGHT;
        } else {
            moves.emplace_back(pos, forward);

            // Double move from starting position
            if (pos.y == startRank) {
                Position doubleForward(pos.x, pos.y + 2 * direction);
                if (doubleForward.IsValid() && IsEmpty(doubleForward)) {
                    moves.emplace_back(pos, doubleForward);
                }
            }
        }
    }

    // Capture moves
    for (int dx : {-1, 1}) {
        Position capture(pos.x + dx, pos.y + direction);
        if (capture.IsValid()) {
            const Piece& target = GetPiece(capture);
            if (!target.IsEmpty() && target.color != color) {
                if (capture.y == promotionRank) {
                    // Promotion captures
                    moves.emplace_back(pos, capture, MoveType::PROMOTION);
                    moves.back().promotionPiece = PieceType::QUEEN;
                    moves.emplace_back(pos, capture, MoveType::PROMOTION);
                    moves.back().promotionPiece = PieceType::ROOK;
                    moves.emplace_back(pos, capture, MoveType::PROMOTION);
                    moves.back().promotionPiece = PieceType::BISHOP;
                    moves.emplace_back(pos, capture, MoveType::PROMOTION);
                    moves.back().promotionPiece = PieceType::KNIGHT;
                } else {
                    moves.emplace_back(pos, capture);
                }
            }
            // En passant
            else if (capture == enPassantTarget) {
                moves.emplace_back(pos, capture, MoveType::EN_PASSANT);
            }
        }
    }

    // Filter out moves that leave king in check
    moves.erase(std::remove_if(moves.begin(), moves.end(),
        [this, color](const Move& move) {
            return WouldBeInCheck(move, color);
        }), moves.end());

    return moves;
}

std::vector<Move> Board::GetRookMoves(const Position& pos) const {
    const std::vector<Position> directions = {
        Position(-1, 0), Position(1, 0), Position(0, -1), Position(0, 1)
    };
    auto moves = GetSlidingMoves(pos, directions);

    // Filter out moves that leave king in check
    const Piece& piece = GetPiece(pos);
    moves.erase(std::remove_if(moves.begin(), moves.end(),
        [this, piece](const Move& move) {
            return WouldBeInCheck(move, piece.color);
        }), moves.end());

    return moves;
}

std::vector<Move> Board::GetKnightMoves(const Position& pos) const {
    std::vector<Move> moves;
    const Piece& piece = GetPiece(pos);

    const std::array<Position, 8> knightMoves = {
        Position(-2, -1), Position(-2, 1), Position(-1, -2), Position(-1, 2),
        Position(1, -2), Position(1, 2), Position(2, -1), Position(2, 1)
    };

    for (const auto& offset : knightMoves) {
        Position target(pos.x + offset.x, pos.y + offset.y);
        if (target.IsValid()) {
            const Piece& targetPiece = GetPiece(target);
            if (targetPiece.IsEmpty() || targetPiece.color != piece.color) {
                moves.emplace_back(pos, target);
            }
        }
    }

    // Filter out moves that leave king in check
    moves.erase(std::remove_if(moves.begin(), moves.end(),
        [this, piece](const Move& move) {
            return WouldBeInCheck(move, piece.color);
        }), moves.end());

    return moves;
}

std::vector<Move> Board::GetBishopMoves(const Position& pos) const {
    const std::vector<Position> directions = {
        Position(-1, -1), Position(-1, 1), Position(1, -1), Position(1, 1)
    };
    auto moves = GetSlidingMoves(pos, directions);

    // Filter out moves that leave king in check
    const Piece& piece = GetPiece(pos);
    moves.erase(std::remove_if(moves.begin(), moves.end(),
        [this, piece](const Move& move) {
            return WouldBeInCheck(move, piece.color);
        }), moves.end());

    return moves;
}

std::vector<Move> Board::GetQueenMoves(const Position& pos) const {
    const std::vector<Position> directions = {
        Position(-1, 0), Position(1, 0), Position(0, -1), Position(0, 1),
        Position(-1, -1), Position(-1, 1), Position(1, -1), Position(1, 1)
    };
    auto moves = GetSlidingMoves(pos, directions);

    // Filter out moves that leave king in check
    const Piece& piece = GetPiece(pos);
    moves.erase(std::remove_if(moves.begin(), moves.end(),
        [this, piece](const Move& move) {
            return WouldBeInCheck(move, piece.color);
        }), moves.end());

    return moves;
}

std::vector<Move> Board::GetKingMoves(const Position& pos) const {
    std::vector<Move> moves;
    const Piece& piece = GetPiece(pos);
    Color color = piece.color;
    Color enemyColor = (color == Color::WHITE) ? Color::BLACK : Color::WHITE;

    // Regular king moves
    for (int dx = -1; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && dy == 0) continue;

            Position target(pos.x + dx, pos.y + dy);
            if (target.IsValid()) {
                const Piece& targetPiece = GetPiece(target);
                if (targetPiece.IsEmpty() || targetPiece.color != color) {
                    // Check if moving to this square would put king in check
                    if (!IsSquareAttacked(target, enemyColor)) {
                        moves.emplace_back(pos, target);
                    }
                }
            }
        }
    }

    // Castling moves
    if (!IsInCheck(color)) {
        int homeRank = (color == Color::WHITE) ? 7 : 0;

        // King-side castling
        if (CanCastleKingSide(color)) {
            Position f(5, homeRank), g(6, homeRank);
            if (IsEmpty(f) && IsEmpty(g) &&
                !IsSquareAttacked(f, enemyColor) && !IsSquareAttacked(g, enemyColor)) {
                moves.emplace_back(pos, g, MoveType::CASTLING);
            }
        }

        // Queen-side castling
        if (CanCastleQueenSide(color)) {
            Position b(1, homeRank), c(2, homeRank), d(3, homeRank);
            if (IsEmpty(b) && IsEmpty(c) && IsEmpty(d) &&
                !IsSquareAttacked(c, enemyColor) && !IsSquareAttacked(d, enemyColor)) {
                moves.emplace_back(pos, c, MoveType::CASTLING);
            }
        }
    }

    return moves;
}

std::vector<Move> Board::GetSlidingMoves(const Position& pos, const std::vector<Position>& directions) const {
    std::vector<Move> moves;
    const Piece& piece = GetPiece(pos);

    for (const auto& dir : directions) {
        Position current(pos.x + dir.x, pos.y + dir.y);

        while (current.IsValid()) {
            const Piece& targetPiece = GetPiece(current);

            if (targetPiece.IsEmpty()) {
                moves.emplace_back(pos, current);
            } else {
                if (targetPiece.color != piece.color) {
                    moves.emplace_back(pos, current); // Capture
                }
                break; // Can't move further
            }

            current.x += dir.x;
            current.y += dir.y;
        }
    }

    return moves;
}

bool Board::IsLegalMove(const Move& move) const {
    auto legalMoves = GetPieceMoves(move.from);
    return std::find_if(legalMoves.begin(), legalMoves.end(),
        [&move](const Move& legal) {
            return legal.from == move.from && legal.to == move.to &&
                   legal.type == move.type && legal.promotionPiece == move.promotionPiece;
        }) != legalMoves.end();
}

bool Board::MakeMove(const Move& move) {
    if (!IsLegalMove(move)) {
        return false;
    }

    Piece movingPiece = GetPiece(move.from);
    Piece capturedPiece = GetPiece(move.to);

    UpdateCastlingRights(move, movingPiece, capturedPiece);
    UpdateEnPassant(move, movingPiece);

    // Handle special moves
    switch (move.type) {
        case MoveType::CASTLING: {
            // Move king
            SetPiece(move.to, movingPiece);
            SetPiece(move.from, Piece());

            // Move rook
            int homeRank = (movingPiece.color == Color::WHITE) ? 7 : 0;
            if (move.to.x == 6) { // King-side castling
                SetPiece(Position(5, homeRank), GetPiece(Position(7, homeRank)));
                SetPiece(Position(7, homeRank), Piece());
            } else { // Queen-side castling
                SetPiece(Position(3, homeRank), GetPiece(Position(0, homeRank)));
                SetPiece(Position(0, homeRank), Piece());
            }
            break;
        }
        case MoveType::EN_PASSANT: {
            SetPiece(move.to, movingPiece);
            SetPiece(move.from, Piece());
            // Remove captured pawn
            int capturedRank = (movingPiece.color == Color::WHITE) ? 4 : 3;
            SetPiece(Position(move.to.x, capturedRank), Piece());
            break;
        }
        case MoveType::PROMOTION: {
            SetPiece(move.to, Piece(move.promotionPiece, movingPiece.color));
            SetPiece(move.from, Piece());
            break;
        }
        default: {
            SetPiece(move.to, movingPiece);
            SetPiece(move.from, Piece());
            break;
        }
    }

    if (movingPiece.type == PieceType::PAWN || !capturedPiece.IsEmpty()) {
        ResetHalfMoveClock();
    }
    else {
        IncrementHalfMoveClock();
    }

    if (currentPlayer == Color::BLACK) {
        IncrementFullMoveNumber();
    }

    // Switch player and record position
    SwitchPlayer();
    AddToHistory();

    return true;
}

bool Board::UndoMove(const Move& move) {
    // This is a simplified undo - in a full implementation, you'd store more state
    // For now, we'll implement the basic structure

    // Switch player back
    SwitchPlayer();

    // Restore pieces (this would need to store captured piece info)
    Piece movingPiece = GetPiece(move.to);
    SetPiece(move.from, movingPiece);
    SetPiece(move.to, move.capturedPiece);

    // Handle special moves (simplified)
    switch (move.type) {
        case MoveType::CASTLING: {
            int homeRank = (movingPiece.color == Color::WHITE) ? 7 : 0;
            if (move.to.x == 6) { // King-side castling
                SetPiece(Position(7, homeRank), GetPiece(Position(5, homeRank)));
                SetPiece(Position(5, homeRank), Piece());
            } else { // Queen-side castling
                SetPiece(Position(0, homeRank), GetPiece(Position(3, homeRank)));
                SetPiece(Position(3, homeRank), Piece());
            }
            break;
        }
        case MoveType::EN_PASSANT: {
            int capturedRank = (movingPiece.color == Color::WHITE) ? 4 : 3;
            SetPiece(Position(move.to.x, capturedRank), Piece(PieceType::PAWN,
                     movingPiece.color == Color::WHITE ? Color::BLACK : Color::WHITE));
            break;
        }
        case MoveType::PROMOTION: {
            SetPiece(move.from, Piece(PieceType::PAWN, movingPiece.color));
            break;
        }
        default:
            break;
    }

    return true;
}

GameResult Board::GetGameResult() const {
    auto legalMoves = GetAllLegalMoves(currentPlayer);

    if (legalMoves.empty()) {
        if (IsInCheck(currentPlayer)) {
            return (currentPlayer == Color::WHITE) ? GameResult::CHECKMATE_BLACK : GameResult::CHECKMATE_WHITE;
        } else {
            return GameResult::STALEMATE;
        }
    }

    // Check for draw conditions
    if (halfMoveClock >= 50) {
        return GameResult::DRAW_50_MOVES;
    }

    if (IsThreefoldRepetition()) {
        return GameResult::DRAW_REPETITION;
    }

    if (IsInsufficientMaterial()) {
        return GameResult::DRAW_MATERIAL;
    }

    return GameResult::ONGOING;
}

bool Board::IsStalemate(Color color) const {
    return !IsInCheck(color) && GetAllLegalMoves(color).empty();
}

bool Board::IsCheckmate(Color color) const {
    return IsInCheck(color) && GetAllLegalMoves(color).empty();
}

bool Board::IsInsufficientMaterial() const {
    std::vector<Piece> pieces;
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            const Piece& piece = GetPiece(x, y);
            if (!piece.IsEmpty()) {
                pieces.push_back(piece);
            }
        }
    }

    // King vs King
    if (pieces.size() == 2) return true;

    // King vs King + Knight/Bishop
    if (pieces.size() == 3) {
        for (const auto& piece : pieces) {
            if (piece.type == PieceType::KNIGHT || piece.type == PieceType::BISHOP) {
                return true;
            }
        }
    }

    return false;
}

bool Board::IsThreefoldRepetition() const {
    if (positionHistory.size() < 3) return false;

    std::string currentPos = GetPositionHash();
    int count = 0;
    for (const auto& pos : positionHistory) {
        if (pos == currentPos && ++count >= 3) return true;
    }
    return false;
}

float Board::EvaluatePosition(Color perspective) const {
    float evaluation = 0.0f;

    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            const Piece& piece = GetPiece(x, y);
            if (!piece.IsEmpty()) {
                float pieceValue = GetPieceValue(piece.type);
                float positionValue = GetPositionValue(Position(x, y), piece.type, piece.color);

                float totalValue = pieceValue + positionValue;
                if (piece.color == Color::WHITE) {
                    evaluation += totalValue;
                } else {
                    evaluation -= totalValue;
                }
            }
        }
    }

    // Return from perspective of the given color
    return (perspective == Color::WHITE) ? evaluation : -evaluation;
}

float Board::GetPieceValue(PieceType type) const {
    switch (type) {
        case PieceType::PAWN: return 100.0f;
        case PieceType::KNIGHT: return 320.0f;
        case PieceType::BISHOP: return 330.0f;
        case PieceType::ROOK: return 500.0f;
        case PieceType::QUEEN: return 900.0f;
        case PieceType::KING: return 20000.0f;
        default: return 0.0f;
    }
}

float Board::GetPositionValue(const Position& pos, PieceType type, Color color) const {
    if (!pos.IsValid()) return 0.0f;

    int x = pos.x, y = pos.y;

    // Flip board for black pieces
    if (color == Color::BLACK) {
        y = BOARD_SIZE - 1 - y;
    }

    switch (type) {
        case PieceType::PAWN: return PAWN_TABLE[y][x];
        case PieceType::KNIGHT: return KNIGHT_TABLE[y][x];
        case PieceType::BISHOP: return BISHOP_TABLE[y][x];
        case PieceType::ROOK: return ROOK_TABLE[y][x];
        case PieceType::QUEEN: return QUEEN_TABLE[y][x];
        case PieceType::KING: return KING_TABLE[y][x];
        default: return 0.0f;
    }
}

std::string Board::ToFEN() const {
    std::ostringstream fen;

    // Piece placement
    for (int y = 0; y < BOARD_SIZE; ++y) {
        int emptyCount = 0;
        for (int x = 0; x < BOARD_SIZE; ++x) {
            const Piece& piece = GetPiece(x, y);
            if (piece.IsEmpty()) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen << emptyCount;
                    emptyCount = 0;
                }
                fen << piece.ToChar();
            }
        }
        if (emptyCount > 0) {
            fen << emptyCount;
        }
        if (y < BOARD_SIZE - 1) {
            fen << '/';
        }
    }

    // Active color
    fen << ' ' << (currentPlayer == Color::WHITE ? 'w' : 'b');

    // Castling rights
    fen << ' ';
    std::string castling;
    if (whiteKingSideCastle) castling += 'K';
    if (whiteQueenSideCastle) castling += 'Q';
    if (blackKingSideCastle) castling += 'k';
    if (blackQueenSideCastle) castling += 'q';
    fen << (castling.empty() ? "-" : castling);

    // En passant target
    fen << ' ' << (enPassantTarget.IsValid() ? enPassantTarget.ToAlgebraic() : "-");

    // Halfmove clock and fullmove number
    fen << ' ' << halfMoveClock << ' ' << fullMoveNumber;

    return fen.str();
}

bool Board::LoadFromFEN(const std::string& fen) {
    // Simplified FEN loading - full implementation would be more robust
    std::istringstream ss(fen);
    std::string piecePlacement, activeColor, castlingRights, enPassant;
    int halfmove, fullmove;

    if (!(ss >> piecePlacement >> activeColor >> castlingRights >> enPassant >> halfmove >> fullmove)) {
        return false;
    }

    Clear();

    // Parse piece placement (simplified)
    int x = 0, y = 0;
    for (char c : piecePlacement) {
        if (c == '/') {
            x = 0;
            y++;
        } else if (std::isdigit(c)) {
            x += c - '0';
        } else {
            Color color = std::isupper(c) ? Color::WHITE : Color::BLACK;
            c = std::tolower(c);
            PieceType type;
            switch (c) {
                case 'p': type = PieceType::PAWN; break;
                case 'r': type = PieceType::ROOK; break;
                case 'n': type = PieceType::KNIGHT; break;
                case 'b': type = PieceType::BISHOP; break;
                case 'q': type = PieceType::QUEEN; break;
                case 'k': type = PieceType::KING; break;
                default: return false;
            }
            SetPiece(x, y, Piece(type, color));
            x++;
        }
    }

    // Set other properties
    currentPlayer = (activeColor == "w") ? Color::WHITE : Color::BLACK;

    whiteKingSideCastle = castlingRights.find('K') != std::string::npos;
    whiteQueenSideCastle = castlingRights.find('Q') != std::string::npos;
    blackKingSideCastle = castlingRights.find('k') != std::string::npos;
    blackQueenSideCastle = castlingRights.find('q') != std::string::npos;

    enPassantTarget = (enPassant != "-") ? Position(enPassant) : Position();
    halfMoveClock = halfmove;
    fullMoveNumber = fullmove;

    AddToHistory();
    return true;
}

std::string Board::ToString() const {
    std::ostringstream ss;

    ss << "  +---+---+---+---+---+---+---+---+\n";
    for (int y = 0; y < BOARD_SIZE; ++y) {
        ss << fmt::format("{} |", 8 - y);
        for (int x = 0; x < BOARD_SIZE; ++x) {
            const Piece& piece = GetPiece(x, y);
            char c = piece.IsEmpty() ? ' ' : piece.ToChar();
            ss << fmt::format(" {} |", c);
        }
        ss << "\n  +---+---+---+---+---+---+---+---+\n";
    }
    ss << "    a   b   c   d   e   f   g   h\n";

    return ss.str();
}

std::string Board::GetPositionHash() const {
    std::istringstream ss(ToFEN());
    std::string placement, active, castling, enpass;
    if (!(ss >> placement >> active >> castling >> enpass)) {
        return ToFEN(); // fallback
    }
    // include placement, active color, castling rights and en-passant target
    return placement + " " + active + " " + castling + " " + enpass;
}

void Board::AddToHistory() {
    positionHistory.push_back(GetPositionHash());
}

void Board::ClearHistory() {
    positionHistory.clear();
}

bool Board::IsValidPosition(const Position& pos) const {
    return IsValidPosition(pos.x, pos.y);
}

bool Board::IsValidPosition(int x, int y) const {
    return IsValidSquare(x, y);
}

bool Board::IsValidSquare(int x, int y) {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

bool Board::WouldBeInCheck(const Move& move, Color color) const {
    // Create a temporary board to test the move
    Board tempBoard = *this;

    // Make the move on temporary board
    Piece movingPiece = tempBoard.GetPiece(move.from);
    tempBoard.SetPiece(move.to, movingPiece);
    tempBoard.SetPiece(move.from, Piece());

    // Handle special moves
    if (move.type == MoveType::EN_PASSANT) {
        int capturedRank = (color == Color::WHITE) ? 4 : 3;
        tempBoard.SetPiece(Position(move.to.x, capturedRank), Piece());
    }

    return tempBoard.IsInCheck(color);
}

void Board::UpdateCastlingRights(const Move& move, const Piece& movingPiece, const Piece& capturedPiece) {
    // King moves disable castling for that color
    if (movingPiece.type == PieceType::KING) {
        SetCastlingRights(movingPiece.color, false, false);
    }

    // Rook moves disable castling on that side
    if (movingPiece.type == PieceType::ROOK) {
        if (movingPiece.color == Color::WHITE) {
            if (move.from.x == 0 && move.from.y == 7) whiteQueenSideCastle = false;
            if (move.from.x == 7 && move.from.y == 7) whiteKingSideCastle = false;
        }
        else {
            if (move.from.x == 0 && move.from.y == 0) blackQueenSideCastle = false;
            if (move.from.x == 7 && move.from.y == 0) blackKingSideCastle = false;
        }
    }

    // Rook captures disable castling on the captured side
    if (capturedPiece.type == PieceType::ROOK) {
        if (capturedPiece.color == Color::WHITE) {
            if (move.to.x == 0 && move.to.y == 7) whiteQueenSideCastle = false;
            if (move.to.x == 7 && move.to.y == 7) whiteKingSideCastle = false;
        }
        else {
            if (move.to.x == 0 && move.to.y == 0) blackQueenSideCastle = false;
            if (move.to.x == 7 && move.to.y == 0) blackKingSideCastle = false;
        }
    }
}

void Board::UpdateEnPassant(const Move& move, const Piece& movingPiece) {
    // Clear previous en passant target
    enPassantTarget = Position();

    // If pawn double-moved, set en-passant target square
    if (movingPiece.type == PieceType::PAWN && std::abs(move.to.y - move.from.y) == 2) {
        enPassantTarget = Position(move.from.x, (move.from.y + move.to.y) / 2);
    }
}

} // namespace Chess