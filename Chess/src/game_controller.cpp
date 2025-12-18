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
    , playerColor(controllerColor)
    , isNetworkGame(this->network != nullptr)
    , isAIGame(this->stockfish != nullptr)
    , state(ControllerState::None)
    , aiThinking(false)
{
    this->graphics->setOnResign([this]() { resign(); });

    if (controllerColor == Color::Black)
        state = ControllerState::OpponentTurn;

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

void GameController::setOnGameEnd(std::function<void(void)> onGameEnd)
{
    this->onGameEnd = onGameEnd;
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

    if (isNetworkGame && network && network->isConnected())
    {
        if (network->isPeerResigned())
        {
            gameEnd(playerColor, " (opponent resign)");
            return;
        }

        Move receivedMove = network->receiveMove();

        if (receivedMove.isValid())
        {
            if (state == ControllerState::OpponentTurn)
            {
                std::vector<Move> legalMoves = board->getSelectableMoves(receivedMove.getFrom());
                bool moveFound = false;

                for (const auto& m : legalMoves)
                {
                    if (m == receivedMove)
                    {
                        if (m.isPromotion())
                        {
                            if (m.getPromotionPiece() == receivedMove.getPromotionPiece())
                            {
                                board->makeMove(m);
                                moveFound = true;
                                break;
                            }
                        }
                        else
                        {
                            board->makeMove(m);
                            moveFound = true;
                            break;
                        }
                    }
                }

                if (moveFound)
                {
                    graphics->clearHighlights();
                    graphics->setCellTypeHl(receivedMove.getFrom(), Highlight::LAST_POS);
                    graphics->setCellTypeHl(receivedMove.getTo(), Highlight::CURRENT_POS);

                    state = ControllerState::None; 

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
                else
                {
                    std::cerr << "Error: Illegal move received from network" << std::endl;
                }
            }
            else
            {
                std::cerr << "Warning: Received move during my turn!" << std::endl;
            }
        }

        if (network->isPeerResigned())
        {
            gameEnd(playerColor, " (opponent resign)");
            return;
        }
    }

    if (isAIGame && !aiThinking && aiThread.joinable())
    {
        aiThread.join();
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
                        gameEnd();
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

    if (isAIGame && !aiThinking && state == ControllerState::OpponentTurn)
    {
        state = ControllerState::OpponentTurn;
        aiThinking = true;
        if (aiThread.joinable())
            aiThread.join();
        aiThread = std::thread(&GameController::aiThreadFunc, this);
    }
}

void GameController::aiThreadFunc()
{
    if (stockfish)
    {
        std::string fen = board->getFen();
        std::cout << fen << std::endl;
        std::string move = stockfish->getBestMove(fen);
        std::cout << move << std::endl
                  << std::endl;
        aiBestMoveStr = move;
    }
    // Сигнал о завершении
    aiThinking = false;
}

Move GameController::stringToMove(std::string moveStr)
{
    if (moveStr.length() < 4)
        return Move();

    int fromX = moveStr[0] - 'a';
    int fromY = moveStr[1] - '1';
    int toX = moveStr[2] - 'a';
    int toY = moveStr[3] - '1';

    Position from(fromX, fromY);
    Position to(toX, toY);

    if (!from.isValid() || !to.isValid())
        return Move();

    std::vector<Move> moves = board->getSelectableMoves(from);

    std::string promoType = "";
    if (moveStr.length() == 5)
    {
        char p = moveStr[4];
        if (p == 'q')
            promoType = "queen";
        else if (p == 'r')
            promoType = "rook";
        else if (p == 'b')
            promoType = "bishop";
        else if (p == 'n')
            promoType = "knight";
    }

    for (const auto& m : moves)
    {
        if (m.getTo() == to)
        {
            if (m.isPromotion())
            {
                if (m.getPromotionPiece() == promoType)
                    return m;
            }
            else
            {
                return m;
            }
        }
    }
    return Move();
}

void GameController::onClick(int x, int y)
{
    // Блокируем клики, пока думает ИИ
    if (state == ControllerState::OpponentTurn || state == ControllerState::PromotionWait || afterEnd || aiThinking)
        return;

    auto sfmlGraphics = std::dynamic_pointer_cast<SFMLGraphics>(graphics);
    if (!sfmlGraphics)
        return;

    Position targetPos(x, y);

    if (!targetPos.isValid())
        return;

    auto& grid = board->getGrid();

    graphics->clearHighlights();

    if (state == ControllerState::None)
    {
        auto& piece = grid[targetPos.getY()][targetPos.getX()];
        if (piece && piece->getColor() == board->getCurrentPlayer())
        {
            selectedPos = targetPos;
            state = ControllerState::PieceSelected;
            graphics->setSelectedPiece(targetPos);
            hightLightsMoves = board->getSelectableMoves(targetPos);
            graphics->highlightMoves(hightLightsMoves);
        }
    }
    else if (state == ControllerState::PieceSelected)
    {
        Move move(*selectedPos, targetPos);
        auto hlMove = std::find(hightLightsMoves.begin(), hightLightsMoves.end(), move);

        if (hlMove != hightLightsMoves.end())
        {
            if (hlMove->isPromotion())
            {
                state = ControllerState::PromotionWait;
                Move promotionMove = *hlMove;
                graphics->showPromotionSelector(board->getCurrentPlayer(), [this, promotionMove](std::string pieceType) {
                        Move m(promotionMove.getFrom(), promotionMove.getTo(),
                            promotionMove.isCastling(), true, promotionMove.isCapture(), pieceType);

                        if (board->makeMove(m))
                        {
                            state = (isAIGame || isNetworkGame) ? ControllerState::OpponentTurn : ControllerState::None;
                            selectedPos = std::nullopt;
                            graphics->setCellTypeHl(promotionMove.getFrom(), Highlight::LAST_POS);
                            graphics->setCellTypeHl(promotionMove.getTo(), Highlight::CURRENT_POS);
                            graphics->hidePromotionSelector();
                            GameStatus gst = board->getGameStatus();
                            if (gst == GameStatus::END_GAME)
                            {
                                gameEnd();
                            }
                            else if (gst == GameStatus::CHECK)
                            {
                                board->findPiece("king", board->getCurrentPlayer());
                                graphics->setCellTypeHl(m.getTo(), Highlight::CHECK_POS);
                            }
                        } }, board->getPromotionTypes());
                return;
            }
            else
            {
                if (board->makeMove(*hlMove))
                {
                    // Добавление логики работы с сервером
                    if(isNetworkGame && network)
                    {
                        network->sendMove(*hlMove);
                    }
                    state = (isAIGame || isNetworkGame) ? ControllerState::OpponentTurn : ControllerState::None;
                    selectedPos = std::nullopt;
                    graphics->setCellTypeHl(hlMove->getFrom(), Highlight::LAST_POS);
                    graphics->setCellTypeHl(hlMove->getTo(), Highlight::CURRENT_POS);
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
                    return;
                }
            }
        }

        auto& piece = grid[targetPos.getY()][targetPos.getX()];
        if (piece && piece->getColor() == board->getCurrentPlayer())
        {
            selectedPos = targetPos;
            graphics->setSelectedPiece(targetPos);
            hightLightsMoves = board->getSelectableMoves(targetPos);
            graphics->highlightMoves(hightLightsMoves);
        }
        else
        {
            state = ControllerState::None;
            selectedPos = std::nullopt;
        }
    }
}

void GameController::resign()
{
    if (board->getGameStatus() == GameStatus::END_GAME)
        return;

    if (isNetworkGame && network)
    {
        network->sendGameOver();
    }

    Color winner = (board->getCurrentPlayer() == Color::White) ? Color::Black : Color::White;

    if (isNetworkGame)
        winner = (playerColor == Color::White) ? Color::Black : Color::White;

    gameEnd(winner, " (opponent resign)");
}

void GameController::gameEnd(std::optional<Color> winner, const std::string& reason)
{
    afterEnd = std::make_unique<Clock>(3.f, 0, 0);
    afterEnd->start();
    board->timeStop();
    if (!winner.has_value())
        winner = board->getWinner();

    if (winner.has_value())
        graphics->showMessage((winner == Color::White ? "White win!" : "Black win!") + reason);
    else
        graphics->showMessage("Draw");
}