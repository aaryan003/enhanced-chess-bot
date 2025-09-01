#pragma once

#include "core/Types.h"
#include "core/Board.h"
#include <vector>
#include <chrono>
#include <memory>
#include <functional>

namespace Chess {

// Forward declarations
class Player;
class AIPlayer;

class GameManager {
private:
    Board board;
    GameConfig config;
    std::vector<MoveHistoryEntry> moveHistory;
    GameStats gameStats;

    // Time management
    std::chrono::steady_clock::time_point moveStartTime;
    TimeControl whiteTimeControl;
    TimeControl blackTimeControl;

    // Game state
    GameResult result;
    bool gameStarted;
    bool gamePaused;

    // Players
    std::unique_ptr<Player> whitePlayer;
    std::unique_ptr<Player> blackPlayer;

    // Event callbacks
    std::function<void(const Move&)> onMoveMade;
    std::function<void(GameResult)> onGameEnd;
    std::function<void(Color, std::chrono::milliseconds)> onTimeUpdate;

public:
    GameManager();
    explicit GameManager(const GameConfig& config);
    ~GameManager();

    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    GameManager(GameManager&&) = default;
    GameManager& operator=(GameManager&&) = default;

    // Game setup
    void SetupNewGame(const GameConfig& config);
    void SetupGame(GameMode mode);
    void SetupFromFEN(const std::string& fen);
    void Reset();
    void ResetGame() { Reset(); }

    // Game control
    bool StartGame();
    void PauseGame();
    void ResumeGame();
    void EndGame(GameResult result = GameResult::RESIGNATION);

    // Move handling
    bool MakeMove(const Move& move);
    bool MakeMove(const std::string& algebraic);
    bool MakeMove(const Position& from, const Position& to, PieceType promotion = PieceType::EMPTY);
    void UndoLastMove();
    void UndoMovesToPosition(size_t historyIndex);
    void RequestAIMove();

    // Game state queries
    const Board& GetBoard() const { return board; }
    Board& GetBoard() { return board; }
    GameResult GetGameResult() const { return result; }
    Color GetCurrentPlayer() const { return board.GetCurrentPlayer(); }
    bool IsGameActive() const { return result == GameResult::ONGOING && gameStarted && !gamePaused; }
    bool IsGamePaused() const { return gamePaused; }
    bool IsGameStarted() const { return gameStarted; }

    bool IsInCheck() const;
    bool IsGameOver() const;
    bool IsCheckmate() const;
    bool IsStalemate() const;
    int GetCaptureCount() const { return gameStats.captures; }

    // Move history
    const std::vector<MoveHistoryEntry>& GetMoveHistory() const { return moveHistory; }
    size_t GetMoveCount() const { return moveHistory.size(); }
    const MoveHistoryEntry* GetLastMove() const;

    // Time control
    const TimeControl& GetTimeControl(Color color) const;
    TimeControl& GetTimeControl(Color color);
    std::chrono::milliseconds GetRemainingTime(Color color) const;
    void UpdateTime();

    // Players
    Player* GetPlayer(Color color) const;
    bool IsHumanPlayer(Color color) const;
    bool IsAIPlayer(Color color) const;

    // Move validation and generation
    std::vector<Move> GetLegalMoves() const;
    std::vector<Move> GetLegalMoves(const Position& from) const;
    std::vector<Move> GetValidMovesFromPosition(const Position& from) const;
    bool IsLegalMove(const Move& move) const;
    bool IsValidMove(const Move& move) const { return IsLegalMove(move); }

    // Game analysis
    float GetCurrentEvaluation() const;
    std::string GetGamePGN() const;
    std::string GetCurrentFEN() const;
    const GameStats& GetGameStats() const { return gameStats; }

    // Save/Load
    bool SaveGame(const std::string& filename) const;
    bool LoadGame(const std::string& filename);

    // Event handlers
    void SetOnMoveMade(std::function<void(const Move&)> callback) { onMoveMade = callback; }
    void SetOnGameEnd(std::function<void(GameResult)> callback) { onGameEnd = callback; }
    void SetOnTimeUpdate(std::function<void(Color, std::chrono::milliseconds)> callback) { onTimeUpdate = callback; }

private:
    void InitializePlayers();
    void StartMoveTimer();
    void EndMoveTimer();
    void CheckTimeControl();
    void CheckGameEnd();
    void NotifyMoveMade(const Move& move);
    void NotifyGameEnd();
    void UpdateTimeControls();
    void UpdateGameStats(const Move& move);

    // Algebraic notation
    std::string MoveToAlgebraic(const Move& move) const;
    Move AlgebraicToMove(const std::string& algebraic) const;

    // PGN utilities
    std::string GetPieceSymbol(PieceType type) const;
    std::string DisambiguateMove(const Move& move, const std::vector<Move>& legalMoves) const;
};

} // namespace Chess
