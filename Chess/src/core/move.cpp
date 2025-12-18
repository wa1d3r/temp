#include "move.h"

Move::Move()
{
}

Move::Move(Position from, Position to, bool is_castling, bool is_promotion, bool is_capture, const std::string& promotion_piece)
    : from(from)
    , to(to)
    , is_castling(is_castling)
    , is_promotion(is_promotion)
    , is_capture(is_capture)
    , promotion_piece(promotion_piece)
{
}

Move::Move(const Move& move)
    : from(move.from)
    , to(move.to)
    , is_castling(move.is_castling)
    , is_promotion(move.is_promotion)
    , is_capture(move.is_capture)
    , promotion_piece(move.promotion_piece)
{
}

Move::Move(Move&& move) noexcept
    : from(move.from)
    , to(move.to)
    , is_castling(move.is_castling)
    , is_promotion(move.is_promotion)
    , is_capture(move.is_capture)
    , promotion_piece(move.promotion_piece)
{
}

bool Move::isValid() const { return from.isValid() && to.isValid(); }
bool Move::isCastling() const { return is_castling; }
bool Move::isPromotion() const { return is_promotion; }
bool Move::isCapture() const { return is_capture; }
Position Move::getFrom() const { return from; }
Position Move::getTo() const { return to; }
std::string Move::getPromotionPiece() const { return promotion_piece; }

Move& Move::operator=(const Move& other) noexcept
{
    from = other.from;
    to = other.to;
    is_castling = other.is_castling;
    is_promotion = other.is_promotion;
    is_capture = other.is_capture;
    promotion_piece = other.promotion_piece;

    return *this;
}

bool Move::operator==(const Move& other) const
{
    return to == other.to && from == other.from;
}

std::ostream& operator<<(std::ostream& os, const Move& m)
{
    os << "Move{from: " << m.from.getX() << ' ' << m.from.getY() << ", to: " << m.to.getX() << ' ' << m.to.getY() << "}";
    return os;
}

sf::Packet& operator<<(sf::Packet& packet, const Move& move)
{
    return packet << move.getFrom()
        << move.getTo()
        << move.isCastling()
        << move.isPromotion()
        << move.isCapture()
        << move.getPromotionPiece();
}

sf::Packet& operator>>(sf::Packet& packet, Move& move)
{
    Position from, to;
    bool isCastling, isPromotion, isCapture;
    std::string promoPiece;

    if (packet >> from >> to >> isCastling >> isPromotion >> isCapture >> promoPiece)
    {
        move = Move(from, to, isCastling, isPromotion, isCapture, promoPiece);
    }

    return packet;
}