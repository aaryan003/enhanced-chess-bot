#include "core/Types.h"
#include <stdexcept>
#include <sstream>
#include <cctype>

namespace Chess {

// Position implementation
Position::Position(const std::string& algebraic) {
    if (algebraic.length() != 2) {
        x = y = -1;
        return;
    }

    char file = algebraic[0];
    char rank = algebraic[1];

    if (file >= 'a' && file <= 'h' && rank >= '1' && rank <= '8') {
        x = file - 'a';
        y = '8' - rank;  // Convert to 0-based, top-to-bottom
    } else {
        x = y = -1;
    }
}

std::string Position::ToAlgebraic() const {
    if (!IsValid()) return "";

    std::string result;
    result += static_cast<char>('a' + x);
    result += static_cast<char>('8' - y);
    return result;
}

// Piece implementation
int8_t Piece::ToLegacyValue() const {
    if (type == PieceType::EMPTY) return 0;

    int8_t value = static_cast<int8_t>(type);
    return (color == Color::WHITE) ? value : -value;
}

Piece Piece::FromLegacyValue(int8_t value) {
    if (value == 0) return Piece();

    Color c = (value > 0) ? Color::WHITE : Color::BLACK;
    PieceType t = static_cast<PieceType>(std::abs(value));
    return Piece(t, c);
}

std::string Piece::ToString() const {
    if (type == PieceType::EMPTY) return "EMPTY";

    std::string result;
    result += (color == Color::WHITE) ? "W_" : "B_";

    switch (type) {
        case PieceType::PAWN: result += "PAWN"; break;
        case PieceType::ROOK: result += "ROOK"; break;
        case PieceType::KNIGHT: result += "KNIGHT"; break;
        case PieceType::BISHOP: result += "BISHOP"; break;
        case PieceType::QUEEN: result += "QUEEN"; break;
        case PieceType::KING: result += "KING"; break;
        default: result += "UNKNOWN"; break;
    }

    return result;
}

char Piece::ToChar() const {
    if (type == PieceType::EMPTY) return '.';

    char baseChar;
    switch (type) {
        case PieceType::PAWN: baseChar = 'p'; break;
        case PieceType::ROOK: baseChar = 'r'; break;
        case PieceType::KNIGHT: baseChar = 'n'; break;
        case PieceType::BISHOP: baseChar = 'b'; break;
        case PieceType::QUEEN: baseChar = 'q'; break;
        case PieceType::KING: baseChar = 'k'; break;
        default: return '?';
    }

    return (color == Color::WHITE) ? static_cast<char>(std::toupper(baseChar)) : baseChar;
}

// Move implementation
std::string Move::ToAlgebraic() const {
    return from.ToAlgebraic() + to.ToAlgebraic();
}

std::string Move::ToUCI() const {
    std::string result = from.ToAlgebraic() + to.ToAlgebraic();

    if (type == MoveType::PROMOTION) {
        switch (promotionPiece) {
            case PieceType::QUEEN: result += "q"; break;
            case PieceType::ROOK: result += "r"; break;
            case PieceType::BISHOP: result += "b"; break;
            case PieceType::KNIGHT: result += "n"; break;
            default: break;
        }
    }

    return result;
}

} // namespace Chess