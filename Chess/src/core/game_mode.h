#pragma once
#include "move.h"
#include "pieces.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>
#include <vector>

class GameMode
{
protected:
    std::vector<std::string> promotionTypes = { "queen", "rook", "bishop", "knight" };

public:
    virtual void initializeBoard(Piece::board_type& board) = 0;
    virtual bool isValidMove(const Piece::board_type& board, Color color, Move move, std::optional<Move> lastMove);
    virtual bool isCheckmate(const Piece::board_type& board, Color color, std::optional<Move> lastMove);
    virtual bool ceilInCheck(const Piece::board_type& board, std::vector<Move> enemy_moves, Position pos) const;
    virtual bool isInCheck(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const;
    virtual std::vector<Move> getAllMoves(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const;
    virtual bool isStalemate(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const;
    virtual void move(Piece::board_type& board, Move move, std::optional<Color> currentPlayer);
    virtual const std::vector<std::string>& getPromotionTypes() const;
    virtual std::unique_ptr<Piece> clonePiece(const Piece& piece) const;
    virtual Piece::board_type copyBoard(const Piece::board_type& board) const;
    virtual ~GameMode() = default;
};

class Ñlassic : public GameMode
{
public:
    virtual void initializeBoard(Piece::board_type& board) override;
};

class Fischer : public GameMode
{
    int seed;

public:
    Fischer(int seed = 0)
        : seed(seed)
    {
    }
    virtual void initializeBoard(Piece::board_type& board) override;
    virtual bool isValidMove(const Piece::board_type& board, Color color, Move move, std::optional<Move> lastMove) override;
    virtual void move(Piece::board_type& board, Move move, std::optional<Color> currentPlayer = std::nullopt) override;
    virtual ~Fischer() = default;
};