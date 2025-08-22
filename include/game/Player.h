#pragma once

#include "core/Types.h"
#include "core/Board.h"
#include <string>
#include <chrono>
#include <memory>

namespace Chess {

// Base player class
class Player {
protected:
    std::string name;
    Color color;
    PlayerConfig config;

public:
    Player(const std::string& playerName, Color playerColor, const PlayerConfig& playerConfig);
    virtual ~Player() = default;

    // Basic properties
    const std::string& GetName() const { return name; }
    Color GetColor() const { return color; }
    const PlayerConfig& GetConfig() const { return config; }

    // Pure virtual methods
    virtual bool IsHuman() const = 0;
    virtual Move GetMove(const Board& board, std::chrono::milliseconds timeLimit) = 0;

    // Optional overrides
    virtual void OnGameStart() {}
    virtual void OnGameEnd(GameResult result) {}
    virtual void OnOpponentMove(const Move& move) {}
    virtual void OnTimeUpdate(std::chrono::milliseconds remainingTime) {}

    // Time management
    virtual bool NeedsTimeToThink() const { return false; }
    virtual void StopThinking() {}
};

// Human player class
class HumanPlayer : public Player {
private:
    Move pendingMove;
    bool hasPendingMove;

public:
    HumanPlayer(const std::string& name, Color color, const PlayerConfig& config = PlayerConfig());

    bool IsHuman() const override { return true; }
    Move GetMove(const Board& board, std::chrono::milliseconds timeLimit) override;

    // Human-specific methods
    void SetMove(const Move& move);
    bool HasPendingMove() const { return hasPendingMove; }
    void ClearPendingMove() { hasPendingMove = false; }
};

// AI player base class
class AIPlayer : public Player {
protected:
    Difficulty difficulty;
    bool isThinking;
    std::chrono::steady_clock::time_point thinkingStartTime;

public:
    AIPlayer(const std::string& name, Color color, Difficulty diff, const PlayerConfig& config = PlayerConfig());

    bool IsHuman() const override { return false; }
    Move GetMove(const Board& board, std::chrono::milliseconds timeLimit) override = 0;

    bool NeedsTimeToThink() const override { return true; }
    void StopThinking() override { isThinking = false; }

    // AI-specific properties
    Difficulty GetDifficulty() const { return difficulty; }
    void SetDifficulty(Difficulty diff) { difficulty = diff; }
    bool IsThinking() const { return isThinking; }

protected:
    void StartThinking();
    void StopThinkingInternal();
    std::chrono::milliseconds GetThinkingTime() const;
};

// Factory function
std::unique_ptr<Player> CreatePlayer(const PlayerConfig& config, Color color);

} // namespace Chess