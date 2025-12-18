#include "pieces.h"

bool isHold(Position pos, const Piece::board_type& grid)
{
    return grid[pos.getY()][pos.getX()] != nullptr;
}

const Piece& getPiece(Position pos, const Piece::board_type& grid)
{
    return *grid[pos.getY()][pos.getX()];
}

std::vector<Move> Piece::getSlideMoves(
    const board_type& grid,
    std::vector<std::pair<int, int>> directions) const
{
    std::vector<Move> moves;
    for (const auto& direct : directions)
    {
        bool exit = false;
        for (int i = 1; i < 8; i++)
        {
            if (exit)
                break;

            Position to(pos.getX() + direct.first * i, pos.getY() + direct.second * i);
            if (!to.isValid() || (isHold(to, grid) && getPiece(to, grid).getColor() == color))
                break;
            exit = nullptr != grid[to.getY()][to.getX()];
            moves.push_back(Move(pos, to));
        }
    }
    return moves;
}

Piece::Piece(Color color, const std::string& type, Position pos, bool is_moved=false)
    : color(color)
    , type(type)
    , is_moved(is_moved)
    , pos(pos)
{
}

std::string Piece::getType() const { return type; }

void Piece::move(Move move) {
    pos = move.getTo();
    is_moved = true;
}

Move Piece::getLastMove() const { return lastMove; }

Position Piece::getPosition() const { return pos; }

bool Piece::isMoved() const { return is_moved; }

Color Piece::getColor() const { return color; }

Pawn::Pawn(Color color, Position pos, bool is_moved)
    : Piece(color, "pawn", pos, is_moved)
{
}

std::vector<Move> Pawn::getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const
{
    std::vector<Move> moves;
    int direction = (Color::White == color) ? 1 : -1;
    int capture_moves[] = { -1, 1 };

    Position short_pos = Position(pos.getX(), pos.getY() + direction);
    bool isPromo = (short_pos.getY() == 0 || short_pos.getY() == 7);

    if (short_pos.isValid() && !isHold(short_pos, grid))
    {
        moves.push_back(Move(pos, short_pos, false, isPromo));

        Position long_pos(pos.getX(), pos.getY() + 2 * direction);
        if (!is_moved && long_pos.isValid() && !isHold(long_pos, grid))
        {
            isPromo = (long_pos.getY() == 0 || long_pos.getY() == 7);
            moves.push_back(Move(pos, long_pos, false, isPromo));
        }
    }

    for (int dx : capture_moves)
    {
        Position to_capture = Position(pos.getX() + dx, pos.getY() + direction);

        if (!to_capture.isValid())
            continue;

        if (isHold(to_capture, grid)
            && getPiece(to_capture, grid).getColor() != color)
        {
            isPromo = (to_capture.getY() == 0 || to_capture.getY() == 7);
            moves.push_back(Move(pos, to_capture, false, isPromo));
        }
        
        Position enemyPos = Position(pos.getX() + dx, pos.getY());
        if (lastMove.has_value() && isHold(enemyPos, grid)
            && getPiece(enemyPos, grid).getColor() != color
            && getPiece(enemyPos, grid).getType() == "pawn"
            && lastMove->getTo() == enemyPos)
        {
            if (abs(lastMove->getFrom().getY() - lastMove->getTo().getY()) == 2)
            {
                moves.push_back(Move(pos, to_capture, false, false, true));
            }
        }
    }

    return moves;
}

Rook::Rook(Color color, Position pos, bool is_moved)
    : Piece(color, "rook", pos, is_moved)
{
}

std::vector<Move> Rook::getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const
{
    std::vector<std::pair<int, int>> directions = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
    };
    return getSlideMoves(grid, directions);
}

Bishop::Bishop(Color color, Position pos, bool is_moved)
    : Piece(color, "bishop", pos, is_moved)
{
}

std::vector<Move> Bishop::getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const
{
    std::vector<std::pair<int, int>> directions = {
        { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 }
    };
    return getSlideMoves(grid, directions);
}

Knight::Knight(Color color, Position pos, bool is_moved)
    : Piece(color, "knight", pos, is_moved)
{
}

std::vector<Move> Knight::getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const
{
    std::vector<Move> moves;
    std::vector<std::pair<int, int>> knight_moves = {
        { 2, 1 }, { 2, -1 }, { -2, 1 }, { -2, -1 },
        { 1, 2 }, { 1, -2 }, { -1, 2 }, { -1, -2 }
    };

    for (const auto& move : knight_moves)
    {
        Position to(pos.getX() + move.first, pos.getY() + move.second);
        if (to.isValid() && (!isHold(to, grid) || getPiece(to, grid).getColor() != color))
        {
            moves.push_back(Move(pos, to));
        }
    }
    return moves;
}

Queen::Queen(Color color, Position pos, bool is_moved)
    : Piece(color, "queen", pos, is_moved)
{
}

std::vector<Move> Queen::getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const
{
    std::vector<std::pair<int, int>> directions = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
        { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 }
    };
    return getSlideMoves(grid, directions);
}

King::King(Color color, Position pos, bool is_moved)
    : Piece(color, "king", pos, is_moved)
{
}

std::vector<Move> King::getPossibleMoves(const board_type& grid, std::optional<Move> lastMove) const
{
    std::vector<Move> moves;
    std::vector<std::pair<int, int>> king_moves = {
        { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 },
        { 1, 1 }, { 1, -1 }, { -1, 1 }, { -1, -1 }
    };

    for (const auto& move : king_moves)
    {
        Position to(pos.getX() + move.first, pos.getY() + move.second);
        if (to.isValid() && (!isHold(to, grid) || getPiece(to, grid).getColor() != color))
        {
            moves.push_back(Move(pos, to));
        }
    }

    moves.push_back(Move(pos, Position(6, pos.getY()), true));
    moves.push_back(Move(pos, Position(2, pos.getY()), true));

    return moves;
}