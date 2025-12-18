#pragma once
#include <SFML/Network/Packet.hpp>

class Position
{
    int x;
    int y;

public:
    Position();
    Position(int x, int y);
    Position(const Position&);
    Position(Position&&) noexcept;
    Position& operator=(const Position& other);
    bool isValid() const;
    int getX() const;
    int getY() const;
    bool operator==(const Position& other) const;
    ~Position() = default;
};

sf::Packet& operator<<(sf::Packet& packet, const Position& pos);
sf::Packet& operator>>(sf::Packet& packet, Position& pos);