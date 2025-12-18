#pragma once
#include "Clock.h"
#include "game_mode.h"
#include "move.h"
#include "pieces.h"
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

enum class GameStatus
{
    END_GAME,
    CHECK,
    IN_GAME
};

class Board
{
    std::vector<std::vector<std::unique_ptr<Piece>>> grid;
    std::unique_ptr<GameMode> game_mode;
    std::vector<Move> history;
    Color current_player;
    std::unique_ptr<Clock> clock;

public:
    Board(std::unique_ptr<GameMode> game_mode, float startTimeSeconds, float inc);

    bool makeMove(const Move& move);
    bool isValidMove(const Move& move) const;
    Color getCurrentPlayer() const;
    void switchPlayer();
    GameStatus getGameStatus() const;
    std::optional<Color> getWinner() const;
    std::vector<Move> getCurrentPlayerMoves() const;
    const std::vector<Move>& getHistory() const;
    std::optional<Move> getLastMove() const;
    std::vector<Move> getSelectableMoves(Position pos) const;
    Position findPiece(const std::string& pieceType, Color color);
    const std::vector<std::string>& getPromotionTypes() const;
    const std::vector<std::vector<std::unique_ptr<Piece>>>& getGrid() const;
    void updateClock();
    float getBlackTime() const;
    float getWhiteTime() const;
    bool isTimeUp() const;
    void timeStop();
    std::string getFen() const;

    friend std::ostream& operator<<(std::ostream& os, const Board& board);
};