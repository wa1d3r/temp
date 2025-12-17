#pragma once
#include "move.h"
#include "pieses.h"
#include <vector>

class GameMode
{
public:
    virtual void initializeBoard(Piece::board_type& board) = 0;
    // virtual bool move(Move& move) = 0;
    // virtual std::vector<Move> getAllMoves(const std::vector<std::vector<Piece*>>& board, Color color) = 0;
    virtual bool isValidMove(const Piece::board_type& board, Color color, Move move, std::optional<Move> lastMove) = 0;
    virtual bool isCheckmate(const Piece::board_type& board, Color color, std::optional<Move> lastMove) = 0;
    virtual bool ceilInCheck(const Piece::board_type& board, std::vector<Move> enemy_moves, Position pos) const = 0;
    virtual bool isInCheck(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const = 0;
    virtual std::vector<Move> getAllMoves(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const = 0;
    virtual bool isStalemate(const Piece::board_type& board, Color color, std::optional<Move> lastMove) = 0;
    virtual void move(Piece::board_type& board, Move move, std::optional<Color> currentPlayer) = 0;
    virtual const std::vector<std::string>& getPromotionTypes() const = 0;
    virtual ~GameMode() = default;
};

class Test : public GameMode
{
    std::vector<std::string> promotionTypes = { "queen", "rook", "bishop", "knight" };

public:
    virtual void initializeBoard(Piece::board_type& board) override;
    // virtual bool move(Move& move) = 0;
    // virtual std::vector<Move> getAllMoves(const std::vector<std::vector<Piece*>>& board, Color color) = 0;
    virtual bool isValidMove(const Piece::board_type& board, Color color, Move move, std::optional<Move> lastMove) override;
    virtual bool isCheckmate(const Piece::board_type& board, Color color, std::optional<Move> lastMove) override;
    virtual bool ceilInCheck(const Piece::board_type& board, std::vector<Move> enemy_moves, Position pos) const override;
    virtual bool isInCheck(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const override;
    virtual std::vector<Move> getAllMoves(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const override;
    virtual bool isStalemate(const Piece::board_type& board, Color color, std::optional<Move> lastMove) override;
    virtual void move(Piece::board_type& board, Move move, std::optional<Color> currentPlayer = std::nullopt) override;
    virtual const std::vector<std::string>& getPromotionTypes() const override;
    virtual ~Test() = default;
    std::unique_ptr<Piece> clonePiece(const Piece& piece) const;
    Piece::board_type copyBoard(const Piece::board_type& board) const;
};