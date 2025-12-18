#pragma once
#include "core/Stockfish.h"
#include "core/board.h"
#include "game_interfaces.h"
#include <algorithm>
#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <thread>

enum class ControllerState
{
    None,
    PieceSelected,
    OpponentTurn,
    PromotionWait
};

class GameController
{
private:
    std::unique_ptr<Board> board;
    std::shared_ptr<IGraphicsInterface> graphics;
    std::unique_ptr<INetworkInterface> network;
    std::unique_ptr<Stockfish> stockfish;

    bool isNetworkGame;
    bool isAIGame;

    ControllerState state;
    std::optional<Position> selectedPos;
    std::vector<Move> hightLightsMoves;
    std::function<void(void)> onGameEnd;
    std::unique_ptr<Clock> afterEnd;

    // Многопоточность std
    std::thread aiThread;
    std::atomic<bool> aiThinking;
    std::string aiBestMoveStr;

    // Храним цвет, за который играем
    Color playerColor;

    void aiThreadFunc();

public:
    GameController(std::unique_ptr<Board> board,
        std::shared_ptr<IGraphicsInterface> graphics,
        std::unique_ptr<INetworkInterface> network = nullptr,
        std::unique_ptr<Stockfish> stockfish = nullptr,
        Color controllerColor = Color::White);

    ~GameController(); // Важно для корректного завершения потока

    void setOnGameEnd(std::function<void(void)> onGameEnd);
    void update();
    void resign();
    void onClick(int x, int y);

private:
    void gameEnd(std::optional<Color> winner = std::nullopt, const std::string& reason = "");
    Move stringToMove(std::string moveStr);
};