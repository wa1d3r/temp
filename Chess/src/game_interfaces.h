#pragma once
#include "core/move.h"
#include "core/position.h"
#include "core/board.h"
#include <functional>

enum class Highlight
{
    NO_HIGHLIGHT,
    FRAME,
    POINT,
    CURRENT_POS,
    LAST_POS,
    CHECK_POS
};

class IGraphicsInterface
{
public:
    virtual void drawBoard(const Board& board, Color currentPlayer) = 0;
    virtual void highlightMoves(const std::vector<Move>& moves) = 0;
    virtual void clearHighlights() = 0;
    virtual void setSelectedPiece(Position pos) = 0;
    virtual void showPromotionSelector(Color color, std::function<void(std::string)> callback, const std::vector<std::string>& promotionTypes) = 0;
    virtual void setCellTypeHl(Position pos, Highlight hl) = 0;
    virtual void showMessage(const std::string& message) = 0;
    virtual void setOnResign(std::function<void()> callback) = 0;
    virtual void hidePromotionSelector() = 0;

    virtual ~IGraphicsInterface() = default;
};


class INetworkInterface
{
public:
    virtual void sendMove(const Move& move) = 0;
    virtual Move receiveMove() = 0;
    virtual bool isConnected() = 0;
    virtual void waitForOpponentMove() = 0;

    virtual ~INetworkInterface() = default;
};