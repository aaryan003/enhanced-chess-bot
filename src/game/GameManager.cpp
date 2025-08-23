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

    void GameManager::SetupFromFEN(const std::string& fen) {
        board.LoadFromFEN(fen);
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

        UpdateTime();

        bool success = board.MakeMove(move);
        if (success) {
            // Get the fullMoveNumber from the board before it's incremented
            int currentFullMoveNumber = board.GetFullMoveNumber();
            if (board.GetCurrentPlayer() == Color::BLACK) { // The full move is incremented after black moves
                currentFullMoveNumber--;
            }

            // Create a new MoveHistoryEntry with the correct move number
            MoveHistoryEntry entry(move, move.ToAlgebraic(), std::chrono::milliseconds(0), board.EvaluatePosition(Color::WHITE), currentFullMoveNumber);
            moveHistory.emplace_back(entry);

            CheckGameEnd();
            if (onMoveMade) onMoveMade(move);

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

    bool GameManager::IsHumanPlayer(Color color) const {
        Player* p = GetPlayer(color);
        return p && p->IsHuman();
    }

    bool GameManager::IsAIPlayer(Color color) const {
        Player* p = GetPlayer(color);
        return p && !p->IsHuman();
    }

    void GameManager::RequestAIMove() {
        if (IsGameActive() && IsAIPlayer(board.GetCurrentPlayer())) {
            Move aiMove = GetPlayer(board.GetCurrentPlayer())->GetMove(board, GetTimeControl(board.GetCurrentPlayer()).remainingTime);
            MakeMove(aiMove);
        }
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
                    EndGame(GameResult::TIMEOUT_BLACK);
                }
                if (onTimeUpdate) onTimeUpdate(Color::WHITE, whiteTimeControl.remainingTime);
            } else {
                blackTimeControl.remainingTime -= elapsed;
                blackTimeControl.remainingTime += blackTimeControl.increment;
                if (blackTimeControl.remainingTime <= std::chrono::milliseconds::zero()) {
                    EndGame(GameResult::TIMEOUT_WHITE);
                }
                if (onTimeUpdate) onTimeUpdate(Color::BLACK, blackTimeControl.remainingTime);
            }
        }
    }

    std::string GameManager::GetGamePGN() const {
        std::string pgn;
        for (const auto& entry : moveHistory) {
            if (entry.move.from.y == 6 && entry.move.to.y == 4) { // Fix for pawn moves that may lack fullMoveNumber
                pgn += std::to_string(entry.fullMoveNumber) + ". ";
                if (entry.move.from.x != entry.move.to.x) {
                    pgn += entry.move.from.ToAlgebraic().substr(0,1) + "x";
                }
                pgn += entry.move.to.ToAlgebraic();
                pgn += " ";
            } else {
                if (entry.fullMoveNumber > 0) {
                     pgn += std::to_string(entry.fullMoveNumber) + ". ";
                }
                pgn += entry.algebraicNotation + " ";
            }
        }
        return pgn;
    }

    void GameManager::CheckGameEnd() {
        result = board.GetGameResult();
        if (result != GameResult::ONGOING && onGameEnd) {
            onGameEnd(result);
        }
    }
}
