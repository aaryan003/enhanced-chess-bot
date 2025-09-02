#include "game/GameManager.h"
#include "game/Player.h"
#include <fmt/format.h>

namespace Chess {

    // FIXED: Default Constructor with proper initialization
    //GameManager::GameManager()
    //    : config(GameMode::HUMAN_VS_HUMAN),  // Initialize config first
    //    result(GameResult::ONGOING),
    //    gameStarted(false),
    //    gamePaused(false),
    //    moveStartTime(std::chrono::steady_clock::now()),
    //    whiteTimeControl(),
    //    blackTimeControl()
    //{
    //    // Now safely initialize board and players
    //    board.SetupStartingPosition();
    //    InitializePlayers();

    //    // Initialize time controls from config
    //    whiteTimeControl = config.whitePlayer.timeControl;
    //    blackTimeControl = config.blackPlayer.timeControl;
    //}

    // FIXED: Config-based Constructor with proper initialization
    GameManager::GameManager(const GameConfig& newConfig)
        : config(newConfig),  // Initialize config first
        result(GameResult::ONGOING),
        gameStarted(false),
        gamePaused(false),
        moveStartTime(std::chrono::steady_clock::now()),
        whiteTimeControl(newConfig.whitePlayer.timeControl),
        blackTimeControl(newConfig.blackPlayer.timeControl)
    {
        // Now safely initialize board and players
        board.SetupStartingPosition();
        InitializePlayers();
    }


    GameManager::~GameManager() = default;

    void GameManager::InitializePlayers() {
        // Check if config is properly initialized before creating players
        whitePlayer = CreatePlayer(config.whitePlayer, Color::WHITE);
        blackPlayer = CreatePlayer(config.blackPlayer, Color::BLACK);
    }

    void GameManager::SetupGame(GameMode mode) {
        GameConfig newConfig(mode);
        SetupNewGame(newConfig);
    }

    void GameManager::SetupNewGame(const GameConfig& newConfig) {
        // This is the main setup function that resets everything.
        this->config = newConfig;

        board.SetupStartingPosition();
        moveHistory.clear();

        result = GameResult::ONGOING;
        gameStarted = false;
        gamePaused = false;

        gameStats = GameStats(); // Reset statistics

        InitializePlayers();

        // Set time controls from the new configuration
        whiteTimeControl = config.whitePlayer.timeControl;
        blackTimeControl = config.blackPlayer.timeControl;
    }
    void GameManager::SetupFromFEN(const std::string& fen) {
        if (!board.LoadFromFEN(fen)) {
            // If FEN loading fails, setup starting position instead
            board.SetupStartingPosition();
        }
        moveHistory.clear();
        result = GameResult::ONGOING;
        gameStarted = false;
        gamePaused = false;
        gameStats = GameStats(); // Reset stats
        InitializePlayers();
        whiteTimeControl = config.whitePlayer.timeControl;
        blackTimeControl = config.blackPlayer.timeControl;
        moveStartTime = std::chrono::steady_clock::now();
    }

    void GameManager::Reset() {
        // Reset simply re-runs the setup with the current configuration.
        SetupNewGame(this->config);
    }

    bool GameManager::StartGame() {
        if (gameStarted) {
            return false; // Game is already in progress
        }
        gameStarted = true;
        gamePaused = false;
        moveStartTime = std::chrono::steady_clock::now(); // Start the clock
        return true;
    }

    void GameManager::PauseGame() {
        if (gameStarted && !gamePaused) {
            gamePaused = true;
            UpdateTime(); // Save current time before pausing
        }
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
            gameStarted = false;
            if (onGameEnd) {
                onGameEnd(result);
            }
        }
    }

    bool GameManager::MakeMove(const Move& move) {
        // Check if the move is valid in the current game state
        if (!IsGameActive() || !board.IsLegalMove(move)) {
            return false;
        }

        // Make a copy of the move to store captured piece info
        Move fullMoveData = move;
        fullMoveData.capturedPiece = board.GetPiece(move.to);

        // Execute the move on the board
        bool success = board.MakeMove(fullMoveData);

        if (success) {
            // Record the move in history
            MoveHistoryEntry entry(
                fullMoveData,
                MoveToAlgebraic(fullMoveData), // You'll need to implement this helper
                std::chrono::milliseconds(0),  // Placeholder for time taken
                board.EvaluatePosition(Color::WHITE),
                board.GetFullMoveNumber()
            );
            moveHistory.push_back(entry);

            // Update stats like captures, checks, etc.
            UpdateGameStats(fullMoveData);

            // Check if the move ended the game
            CheckGameEnd();

            // Reset the clock for the next player's turn
            moveStartTime = std::chrono::steady_clock::now();
        }

        return success;
    }
    bool GameManager::MakeMove(const std::string& algebraic) {
        Move move = AlgebraicToMove(algebraic);
        return MakeMove(move);
    }

    bool GameManager::MakeMove(const Position& from, const Position& to, PieceType promotion) {
        Move move(from, to);
        if (promotion != PieceType::EMPTY) {
            move.type = MoveType::PROMOTION;
            move.promotionPiece = promotion;
        }
        return MakeMove(move);
    }

    void GameManager::UndoLastMove() {
        if (moveHistory.empty()) return;

        const MoveHistoryEntry& lastEntry = moveHistory.back();
        board.UndoMove(lastEntry.move);
        moveHistory.pop_back();

        // Update statistics
        if (!lastEntry.move.capturedPiece.IsEmpty()) {
            gameStats.captures--;
        }
        gameStats.totalMoves--;

        // If a move was undone, the game state should revert to ongoing
        if (result != GameResult::ONGOING) {
            result = GameResult::ONGOING;
            gameStarted = true;
        }

        // Reset the start time for the current player's move
        moveStartTime = std::chrono::steady_clock::now();
    }

    void GameManager::UndoMovesToPosition(size_t historyIndex) {
        while (moveHistory.size() > historyIndex) {
            UndoLastMove();
        }
    }

    void GameManager::RequestAIMove() {
        if (IsGameActive() && IsAIPlayer(board.GetCurrentPlayer())) {
            Player* currentPlayer = GetPlayer(board.GetCurrentPlayer());
            if (currentPlayer) {
                auto timeLimit = GetTimeControl(board.GetCurrentPlayer()).remainingTime;
                Move aiMove = currentPlayer->GetMove(board, timeLimit);
                if (aiMove.IsValid()) {
                    MakeMove(aiMove);
                }
            }
        }
    }

    std::vector<Move> GameManager::GetLegalMoves() const {
        return board.GetAllLegalMoves(board.GetCurrentPlayer());
    }

    std::vector<Move> GameManager::GetLegalMoves(const Position& from) const {
        return GetValidMovesFromPosition(from);
    }

    std::vector<Move> GameManager::GetValidMovesFromPosition(const Position& from) const {
        return board.GetPieceMoves(from);
    }

    bool GameManager::IsLegalMove(const Move& move) const {
        return board.IsLegalMove(move);
    }

    const MoveHistoryEntry* GameManager::GetLastMove() const {
        if (moveHistory.empty()) return nullptr;
        return &moveHistory.back();
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
        }
        else {
            return blackTimeControl.remainingTime;
        }
    }

    Player* GameManager::GetPlayer(Color color) const {
        if (color == Color::WHITE) {
            return whitePlayer.get();
        }
        else if (color == Color::BLACK) {
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

    void GameManager::UpdateTime() {
        if (!IsGameActive()) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - moveStartTime);

        TimeControl& currentTC = (board.GetCurrentPlayer() == Color::WHITE)
            ? whiteTimeControl : blackTimeControl;

        currentTC.remainingTime -= elapsed;

        if (currentTC.remainingTime <= std::chrono::milliseconds::zero()) {
            currentTC.remainingTime = std::chrono::milliseconds::zero();
            EndGame((board.GetCurrentPlayer() == Color::WHITE)
                ? GameResult::TIMEOUT_BLACK : GameResult::TIMEOUT_WHITE);
        }

        if (onTimeUpdate) {
            onTimeUpdate(board.GetCurrentPlayer(), currentTC.remainingTime);
        }
    }

    void GameManager::CheckTimeControl() {
        UpdateTime();
    }

    void GameManager::CheckGameEnd() {
        result = board.GetGameResult();
        if (result != GameResult::ONGOING) {
            NotifyGameEnd();
        }
    }

    void GameManager::NotifyMoveMade(const Move& move) {
        if (onMoveMade) {
            onMoveMade(move);
        }
    }

    void GameManager::NotifyGameEnd() {
        if (result != GameResult::ONGOING && onGameEnd) {
            onGameEnd(result);
        }
    }

    void GameManager::UpdateTimeControls() {
        // Add increment after move is made
        if (board.GetCurrentPlayer() == Color::BLACK) {
            whiteTimeControl.remainingTime += whiteTimeControl.increment;
        }
        else {
            blackTimeControl.remainingTime += blackTimeControl.increment;
        }
    }

    void GameManager::UpdateGameStats(const Move& move) {
        gameStats.totalMoves++;

        if (!move.capturedPiece.IsEmpty()) {
            gameStats.captures++;
        }

        if (board.IsInCheck(board.GetCurrentPlayer())) {
            gameStats.checks++;
        }

        if (move.type == MoveType::CASTLING) {
            gameStats.castles++;
        }
    }

    std::string GameManager::GetGamePGN() const {
        std::string pgn;

        // Add headers
        pgn += "[Event \"Chess Game\"]\n";
        pgn += "[Site \"Local\"]\n";
        pgn += "[Date \"" + std::string("2024.01.01") + "\"]\n"; // You can add date formatting
        pgn += "[White \"" + config.whitePlayer.name + "\"]\n";
        pgn += "[Black \"" + config.blackPlayer.name + "\"]\n";

        // Add result
        std::string resultStr = "*";
        switch (result) {
        case GameResult::CHECKMATE_WHITE: resultStr = "1-0"; break;
        case GameResult::CHECKMATE_BLACK: resultStr = "0-1"; break;
        case GameResult::STALEMATE:
        case GameResult::DRAW_50_MOVES:
        case GameResult::DRAW_REPETITION:
        case GameResult::DRAW_MATERIAL: resultStr = "1/2-1/2"; break;
        default: break;
        }
        pgn += "[Result \"" + resultStr + "\"]\n\n";

        // Add moves
        for (size_t i = 0; i < moveHistory.size(); ++i) {
            const auto& entry = moveHistory[i];
            if (i % 2 == 0) {
                pgn += std::to_string(entry.fullMoveNumber) + ". ";
            }
            pgn += entry.algebraicNotation + " ";
            if ((i + 1) % 2 == 0 || i == moveHistory.size() - 1) {
                pgn += "\n";
            }
        }

        pgn += resultStr;
        return pgn;
    }

    bool GameManager::SaveGame(const std::string& filename) const {
        // Implementation would save PGN to file
        return false;
    }

    bool GameManager::LoadGame(const std::string& filename) {
        // Implementation would load PGN from file
        return false;
    }

    bool GameManager::IsInCheck() const {
        // Pass the call directly to the board object
        return board.IsInCheck(board.GetCurrentPlayer());
    }

    bool GameManager::IsGameOver() const {
        // The game is over if the result is anything other than ONGOING
        return result != GameResult::ONGOING;
    }
    bool GameManager::IsCheckmate() const {
        return board.IsCheckmate(board.GetCurrentPlayer());
    }

    bool GameManager::IsStalemate() const {
        return board.IsStalemate(board.GetCurrentPlayer());
    }

    //std::string GameManager::MoveToAlgebraic(const Move& move) const {
    //    // Basic algebraic notation - full implementation would be more complex
    //    return move.ToAlgebraic();
    //}

    // In GameManager.cpp

    std::string GameManager::MoveToAlgebraic(const Move& move) const {
        // Create a temporary board to simulate the move and check for check/checkmate
        Board tempBoard = board;
        tempBoard.MakeMove(move);

        // Get the color of the player who is now in the new position's turn.
        Color opponentColor = tempBoard.GetCurrentPlayer();

        // Step 1: Handle special case for castling
        if (move.type == MoveType::CASTLING) {
            // Kingside castling (King moves to g-file)
            if (move.to.x == 6) {
                // FIXED: Convert the first literal to a std::string
                return std::string("O-O") + (tempBoard.IsCheckmate(opponentColor) ? "#" : (tempBoard.IsInCheck(opponentColor) ? "+" : ""));
            }
            // Queenside castling (King moves to c-file)
            if (move.to.x == 2) {
                // FIXED: Convert the first literal to a std::string
                return std::string("O-O-O") + (tempBoard.IsCheckmate(opponentColor) ? "#" : (tempBoard.IsInCheck(opponentColor) ? "+" : ""));
            }
        }

        std::string notation;
        const Piece& movingPiece = board.GetPiece(move.from);

        // Step 2: Add piece prefix (pawns are an exception)
        if (movingPiece.type != PieceType::PAWN) {
            notation += GetPieceSymbol(movingPiece.type);
        }

        // Step 3: Handle Disambiguation
        if (movingPiece.type != PieceType::PAWN && movingPiece.type != PieceType::KING) {
            std::vector<Move> allLegalMoves = board.GetAllLegalMoves(movingPiece.color);
            std::vector<Position> ambiguousFromSquares;

            for (const auto& legalMove : allLegalMoves) {
                const Piece& p = board.GetPiece(legalMove.from);
                // Find other pieces of the same type that can move to the same destination
                if (p.type == movingPiece.type && legalMove.to == move.to && legalMove.from != move.from) {
                    ambiguousFromSquares.push_back(legalMove.from);
                }
            }

            if (!ambiguousFromSquares.empty()) {
                bool fileIsDifferent = true;
                bool rankIsDifferent = true;
                for (const auto& pos : ambiguousFromSquares) {
                    if (pos.x == move.from.x) fileIsDifferent = false;
                    if (pos.y == move.from.y) rankIsDifferent = false;
                }

                if (fileIsDifferent) {
                    notation += move.from.ToAlgebraic().substr(0, 1); // Add starting file (e.g., 'a')
                }
                else if (rankIsDifferent) {
                    notation += move.from.ToAlgebraic().substr(1, 1); // Add starting rank (e.g., '1')
                }
                else {
                    notation += move.from.ToAlgebraic(); // Add full starting square (e.g., 'a1')
                }
            }
        }

        // Step 4: Add capture indicator 'x'
        bool isCapture = !board.GetPiece(move.to).IsEmpty() || move.type == MoveType::EN_PASSANT;
        if (isCapture) {
            // For pawn captures, we need the starting file
            if (movingPiece.type == PieceType::PAWN) {
                notation += move.from.ToAlgebraic().substr(0, 1);
            }
            notation += 'x';
        }

        // Step 5: Add destination square
        notation += move.to.ToAlgebraic();

        // Step 6: Add promotion indicator
        if (move.type == MoveType::PROMOTION) {
            notation += "=" + GetPieceSymbol(move.promotionPiece);
        }

        // Step 7: Add check (+) or checkmate (#) suffix
        if (tempBoard.IsCheckmate(opponentColor)) {
            notation += '#';
        }
        else if (tempBoard.IsInCheck(opponentColor)) {
            notation += '+';
        }

        return notation;
    }

    Move GameManager::AlgebraicToMove(const std::string& algebraic) const {
        // Parse algebraic notation - simplified version
        if (algebraic.length() >= 4) {
            Position from(algebraic.substr(0, 2));
            Position to(algebraic.substr(2, 2));
            return Move(from, to);
        }
        return Move();
    }

    std::string GameManager::GetPieceSymbol(PieceType type) const {
        switch (type) {
        case PieceType::KNIGHT: return "N";
        case PieceType::BISHOP: return "B";
        case PieceType::ROOK: return "R";
        case PieceType::QUEEN: return "Q";
        case PieceType::KING: return "K";
        default: return "";
        }
    }

    std::string GameManager::DisambiguateMove(const Move& move, const std::vector<Move>& legalMoves) const {
        // Implementation for disambiguating moves in algebraic notation
        return "";
    }

} // namespace Chess