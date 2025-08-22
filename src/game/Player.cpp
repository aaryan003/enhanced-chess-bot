#include "game/Player.h"

namespace Chess {

    Player::Player(const std::string& playerName, Color playerColor, const PlayerConfig& playerConfig)
        : name(playerName), color(playerColor), config(playerConfig) {}

    HumanPlayer::HumanPlayer(const std::string& name, Color color, const PlayerConfig& config)
        : Player(name, color, config), hasPendingMove(false) {}

    Move HumanPlayer::GetMove(const Board& board, std::chrono::milliseconds timeLimit) {
        // In a real implementation, this would wait for user input
        // For now, return an invalid move
        return Move();
    }

    void HumanPlayer::SetMove(const Move& move) {
        pendingMove = move;
        hasPendingMove = true;
    }

    AIPlayer::AIPlayer(const std::string& name, Color color, Difficulty diff, const PlayerConfig& config)
        : Player(name, color, config), difficulty(diff), isThinking(false) {}

    void AIPlayer::StartThinking() {
        isThinking = true;
        thinkingStartTime = std::chrono::steady_clock::now();
    }

    void AIPlayer::StopThinkingInternal() {
        isThinking = false;
    }

    std::chrono::milliseconds AIPlayer::GetThinkingTime() const {
        if (!isThinking) return std::chrono::milliseconds{0};
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(now - thinkingStartTime);
    }

    std::unique_ptr<Player> CreatePlayer(const PlayerConfig& config, Color color) {
        if (config.isHuman) {
            return std::make_unique<HumanPlayer>(config.name, color, config);
        } else {
            // For now, return nullptr - we'll implement AI players tomorrow
            return nullptr;
        }
    }

} // namespace Chess