#pragma once
#include "move.h"
#include "position.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <optional>

enum class Color
{
    White,
    Black
};

class Piece
{
public:
    using board_type = std::vector<std::vector<std::unique_ptr<Piece>>>;

    Piece() = delete;
    Piece(Color color, const std::string& type, Position pos, bool is_moved);

    std::string getType() const;
    bool isMoved() const;
    Move getLastMove() const;
    Position getPosition() const;
    Color getColor() const;

    void move(Move move);
    virtual std::vector<Move> getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const = 0;

    virtual ~Piece() = default;

protected:
    Color color;
    bool is_moved;
    const std::string type;
    Position pos;
    Move lastMove;
    std::vector<Move> getSlideMoves(
        const board_type& grid,
        std::vector<std::pair<int, int>> directions) const;
};

class Pawn : public Piece
{
public:
    Pawn(Color color, Position pos, bool is_moved);
    virtual std::vector<Move> getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const override;
    virtual ~Pawn() = default;
};

class Rook : public Piece
{
public:
    Rook(Color color, Position pos, bool is_moved);
    virtual std::vector<Move> getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const override;
    virtual ~Rook() = default;
};

class Bishop : public Piece
{
public:
    Bishop(Color color, Position pos, bool is_moved);
    virtual std::vector<Move> getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const override;
    virtual ~Bishop() = default;
};

class Knight : public Piece
{
public:
    Knight(Color color, Position pos, bool is_moved);
    virtual std::vector<Move> getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const override;
    virtual ~Knight() = default;
};

class Queen : public Piece
{
public:
    Queen(Color color, Position pos, bool is_moved);
    virtual std::vector<Move> getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const override;
    virtual ~Queen() = default;
};

class King : public Piece
{
public:
    King(Color color, Position pos, bool is_moved);
    virtual std::vector<Move> getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const override;
    virtual ~King() = default;
};