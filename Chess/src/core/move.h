#pragma once
#include "position.h"
#include <iostream>

class Move
{
    Position from;
    Position to;
    bool is_castling;
    bool is_promotion;
    bool is_capture;
    std::string promotion_piece;

public:
    Move();
    Move(Position from, Position to, bool is_castling = false, 
        bool is_promotion = false, bool is_capture = false, const std::string& promotion_piece = "");
    Move(const Move&);
    Move(Move&&) noexcept;
    bool isValid() const;
    bool isCastling() const;
    bool isPromotion() const;
    bool isCapture() const;
    Position getFrom() const;
    Position getTo() const;
    std::string getPromotionPiece() const;

    Move& operator=(const Move&) noexcept;
    bool operator==(const Move&) const;

    friend std::ostream& operator<<(std::ostream& os, const Move& m);
};