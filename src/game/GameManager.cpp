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
        whiteTimeControl = config.whitePlayer.timeControl;
        blackTimeControl = config.blackPlayer.timeControl;
    }

    bool GameManager::StartGame() {
        if (!gameStarted) {
            gameStarted = true;
            gamePaused = false;
            moveStartTime = std::chrono::steady_clock::now();
            return true;
        }
        return false;
    }

    void GameManager::PauseGame() {
        gamePaused = true;
    }

    void GameManager::ResumeGame() {
        if (gamePaused) {
            gamePaused = false;
            moveStartTime = std::chrono::steady_clock::now();
        }
    }

    void GameManager::EndGame(GameResult gameResult) {
        if (result == GameResult::ONGOING) {
            result = gameResult;
            if (onGameEnd) onGameEnd(result);
        }
    }

    bool GameManager::MakeMove(const Move& move) {
        if (!IsGameActive() || !board.IsLegalMove(move)) {
            return false;
        }

        // Update the clock for the current player
        UpdateTime();

        bool success = board.MakeMove(move);
        if (success) {
            moveHistory.emplace_back(move);
            CheckGameEnd();
            if (onMoveMade) onMoveMade(move);

            // Start the clock for the next player
            if (result == GameResult::ONGOING) {
                moveStartTime = std::chrono::steady_clock::now();
            }
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

    std::chrono::milliseconds GameManager::GetRemainingTime(Color color) const {
        if (color == Color::WHITE) {
            return whiteTimeControl.remainingTime;
        } else {
            return blackTimeControl.remainingTime;
        }
    }

    Player* GameManager::GetPlayer(Color color) const {
        if (color == Color::WHITE) {
            return whitePlayer.get();
        } else if (color == Color::BLACK) {
            return blackPlayer.get();
        }
        return nullptr;
    }

    void GameManager::UpdateTime() {
        if (IsGameActive()) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - moveStartTime);
            moveStartTime = now;

            if (board.GetCurrentPlayer() == Color::WHITE) {
                whiteTimeControl.remainingTime -= elapsed;
                whiteTimeControl.remainingTime += whiteTimeControl.increment;
                if (whiteTimeControl.remainingTime <= std::chrono::milliseconds::zero()) {
                    EndGame(GameResult::CHECKMATE_BLACK);
                }
                if (onTimeUpdate) onTimeUpdate(Color::WHITE, whiteTimeControl.remainingTime);
            } else {
                blackTimeControl.remainingTime -= elapsed;
                blackTimeControl.remainingTime += blackTimeControl.increment;
                if (blackTimeControl.remainingTime <= std::chrono::milliseconds::zero()) {
                    EndGame(GameResult::CHECKMATE_WHITE);
                }
                if (onTimeUpdate) onTimeUpdate(Color::BLACK, blackTimeControl.remainingTime);
            }
        }
    }

    void GameManager::CheckGameEnd() {
        result = board.GetGameResult();
        if (result != GameResult::ONGOING && onGameEnd) {
            onGameEnd(result);
        }
    }

} // namespace Chess
