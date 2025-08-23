#include "game/GameManager.h"
#include "game/Player.h"
#include <fmt/format.h>

namespace Chess {

    GameManager::GameManager() : result(GameResult::ONGOING), gameStarted(false), gamePaused(false) {
        board.SetupStartingPosition();
        InitializePlayers();
    }

    GameManager::GameManager(const GameConfig& config) : GameManager() {
        this->config = config;
        InitializePlayers();
    }

    void GameManager::InitializePlayers() {
        whitePlayer = CreatePlayer(config.whitePlayer, Color::WHITE);
        blackPlayer = CreatePlayer(config.blackPlayer, Color::BLACK);
    }

    void GameManager::SetupNewGame(const GameConfig& newConfig) {
        this->config = newConfig;
        board.SetupStartingPosition();
        moveHistory.clear();
        result = GameResult::ONGOING;
        gameStarted = false;
        gamePaused = false;
        InitializePlayers();
    }

    bool GameManager::StartGame() {
        gameStarted = true;
        gamePaused = false;
        return true;
    }

    void GameManager::PauseGame() {
        gamePaused = true;
    }

    void GameManager::ResumeGame() {
        gamePaused = false;
    }

    bool GameManager::MakeMove(const Move& move) {
        if (!IsGameActive() || !board.IsLegalMove(move)) {
            return false;
        }

        bool success = board.MakeMove(move);
        if (success) {
            moveHistory.emplace_back(move);
            CheckGameEnd();
            if (onMoveMade) onMoveMade(move);
        }
        return success;
    }

    std::vector<Move> GameManager::GetLegalMoves() const {
        return board.GetAllLegalMoves(board.GetCurrentPlayer());
    }

    bool GameManager::IsLegalMove(const Move& move) const {
        return board.IsLegalMove(move);
    }

    std::string GameManager::GetCurrentFEN() const {
        return board.ToFEN();
    }

    float GameManager::GetCurrentEvaluation() const {
        return board.EvaluatePosition(Color::WHITE);
    }

    const TimeControl& GameManager::GetTimeControl(Color color) const {
        return (color == Color::WHITE) ? whiteTimeControl : blackTimeControl;
    }

    TimeControl& GameManager::GetTimeControl(Color color) {
        return (color == Color::WHITE) ? whiteTimeControl : blackTimeControl;
    }

    Player* GameManager::GetPlayer(Color color) const {
        if (color == Color::WHITE) {
            return whitePlayer.get();
        } else if (color == Color::BLACK) {
            return blackPlayer.get();
        }
        return nullptr;
    }

    void GameManager::CheckGameEnd() {
        result = board.GetGameResult();
        if (result != GameResult::ONGOING && onGameEnd) {
            onGameEnd(result);
        }
    }

} // namespace Chess