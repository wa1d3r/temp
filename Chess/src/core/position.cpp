#include "position.h"

Position::Position()
    : x(-1)
    , y(-1)
{
}

Position::Position(int x, int y)
    : x(x)
    , y(y)
{
}

Position::Position(const Position& position)
    : x(position.x)
    , y(position.y)
{
}

Position::Position(Position&& position) noexcept
    : x(position.x)
    , y(position.y)
{
}

Position& Position::operator=(const Position& other) {
    x = other.x;
    y = other.y;

    return *this;
}

bool Position::isValid() const
{
    return (0 <= x) && (x < 8) && (0 <= y) && (y < 8);
}

bool Position::operator==(const Position& other) const
{
    return x == other.x && y == other.y;
}

int Position::getX() const { return x; }
int Position::getY() const { return y; }