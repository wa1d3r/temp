#include "game_controller.h"
#include "graphic/sfml_graphics.h"
#include <algorithm>
#include <iostream>

GameController::GameController(std::unique_ptr<Board> board,
    std::shared_ptr<IGraphicsInterface> graphics,
    std::unique_ptr<INetworkInterface> network,
    std::unique_ptr<Stockfish> stockfish,
    Color controllerColor)
    : board(std::move(board))
    , graphics(std::move(graphics))
    , network(std::move(network))
    , stockfish(std::move(stockfish))
    , isNetworkGame(this->network != nullptr)
    , isAIGame(this->stockfish != nullptr)
    , state(ControllerState::None)
    , aiThinking(false)
{
    this->graphics->setOnResign([this]() { resign(); });

    if (controllerColor == Color::Black)
        state == ControllerState::OpponentTurn;

    if (isAIGame)
    {
        if (!this->stockfish->start())
        {
            std::cerr << "Failed to start Stockfish!" << std::endl;
            isAIGame = false;
        }
    }
}

GameController::~GameController()
{
    if (isAIGame && stockfish)
    {
        stockfish->stop(); // Останавливаем процесс, чтобы разблокировать чтение
    }
    if (aiThread.joinable())
    {
        aiThread.join();
    }
}

void GameController::update()
{
    if (afterEnd)
    {
        afterEnd->update();
        if (afterEnd->isTimeUp())
        {
            if (onGameEnd)
                onGameEnd();
        }
        graphics->drawBoard(*board, board->getCurrentPlayer());
        return;
    }

    if (board->isTimeUp())
        gameEnd();

    board->updateClock();
    graphics->drawBoard(*board, board->getCurrentPlayer());

    if (isNetworkGame && state == ControllerState::OpponentTurn)
    {
        if (network && network->isConnected())
        {
            // network logic
        }
    }

    // Проверка завершения потока ИИ
    if (isAIGame && !aiThinking && aiThread.joinable())
    {
        aiThread.join(); // Завершаем поток и забираем результат

        if (!aiBestMoveStr.empty())
        {
            Move m = stringToMove(aiBestMoveStr);
            if (m.isValid())
            {
                if (board->makeMove(m))
                {
                    graphics->clearHighlights();
                    graphics->setCellTypeHl(m.getFrom(), Highlight::LAST_POS);
                    graphics->setCellTypeHl(m.getTo(), Highlight::CURRENT_POS);

                    GameStatus gst = board->getGameStatus();
                    if (gst == GameStatus::END_GAME)
                    {
                        gameEnd();
                    }
                    else if (gst == GameStatus::CHECK)
                    {
                        Position kingPos = board->findPiece("king", board->getCurrentPlayer());
                        graphics->setCellTypeHl(kingPos, Highlight::CHECK_POS);
                    }
                }
            }
        }

        aiBestMoveStr = "";
        state = ControllerState::None;
    }

    // Логика запуска потока ИИ
    if (isAIGame && !aiThinking && state == ControllerState::OpponentTurn)
    {
        state = ControllerState::OpponentTurn;
        aiThinking = true;

        // Если поток уже был использован, его нужно присоединить перед новым запуском
        if (aiThread.joinable())
            aiThread.join();

        aiThread = std::thread(&GameController::aiThreadFunc, this);
    }
}