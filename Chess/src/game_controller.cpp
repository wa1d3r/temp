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