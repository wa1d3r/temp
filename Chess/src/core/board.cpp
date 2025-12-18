#include "board.h"

Board::Board(std::unique_ptr<GameMode> game_mode, float startTimeSeconds, float inc)
    : game_mode(std::move(game_mode))
    , current_player(Color::White)
{
    this->game_mode->initializeBoard(grid);
    clock = std::make_unique<Clock>(startTimeSeconds, inc, true);
}

bool Board::makeMove(const Move& move)
{
    if (!isValidMove(move))
    {
        return false;
    }

    Position from = move.getFrom();
    Position to = move.getTo();

    if (history.empty())
        clock->start();
    history.push_back(move);

    game_mode->move(grid, move, current_player);

    switchPlayer();

    return true;
}

Position Board::findPiece(const std::string& pieceType, Color color)
{
    for (const auto& row : grid)
    {
        auto it = std::find_if(
            row.begin(),
            row.end(),
            [color, pieceType](const auto& piece) {
                return piece && piece->getColor() == color && piece->getType() == pieceType;
            });
        if (it != row.end())
            return it->get()->getPosition();
    }
    return Position();
}

bool Board::isValidMove(const Move& move) const
{
    if (!move.isValid())
    {
        return false;
    }

    Position from = move.getFrom();
    Position to = move.getTo();

    if (!grid[from.getY()][from.getX()])
    {
        return false;
    }

    if (grid[from.getY()][from.getX()]->getColor() != current_player)
    {
        return false;
    }

    return game_mode->isValidMove(grid, current_player, move, getLastMove());
}

Color Board::getCurrentPlayer() const
{
    return current_player;
}

void Board::switchPlayer()
{
    current_player = (current_player == Color::White) ? Color::Black : Color::White;
    clock->switchTurn();
}

GameStatus Board::getGameStatus() const
{
    return (game_mode->isCheckmate(grid, current_player, getLastMove())
        || game_mode->isStalemate(grid, current_player, getLastMove())
        || clock->isTimeUp())
        ? GameStatus::END_GAME
        : game_mode->isInCheck(grid, current_player, getLastMove()) ? GameStatus::CHECK : GameStatus::IN_GAME;
}

std::optional<Color> Board::getWinner() const
{
    if (getGameStatus() != GameStatus::END_GAME)
    {
        return std::nullopt;
    }

    return (current_player == Color::White) ? Color::Black : Color::White;
}

std::vector<Move> Board::getCurrentPlayerMoves() const
{
    return game_mode->getAllMoves(grid, current_player, getLastMove());
}

const std::vector<Move>& Board::getHistory() const
{
    return history;
}

std::optional<Move> Board::getLastMove() const
{
    return history.empty() ? std::nullopt : std::optional<Move>(history.back());
}

void Board::updateClock()
{
    clock->update();
}

float Board::getBlackTime() const
{
    return clock->getBlackTime();
}

float Board::getWhiteTime() const
{
    return clock->getWhiteTime();
}

bool Board::isTimeUp() const
{
    return clock->isTimeUp();
}

void Board::timeStop()
{
    clock->stop();
}

std::vector<Move> Board::getSelectableMoves(Position pos) const
{
    if (!grid[pos.getY()][pos.getX()])
        return {};

    auto candidates = grid[pos.getY()][pos.getX()]->getPossibleMoves(grid, getLastMove());
    std::vector<Move> validMoves;

    for (const auto& move : candidates)
    {
        if (isValidMove(move))
        {
            validMoves.push_back(move);
        }
    }
    return validMoves;
}

const std::vector<std::string>& Board::getPromotionTypes() const
{
    return game_mode->getPromotionTypes();
}

const Piece::board_type& Board::getGrid() const
{
    return grid;
}

std::string Board::getFen() const
{
    std::stringstream ss;

    // 1. Положение фигур
    for (int y = 7; y >= 0; --y)
    {
        int emptyCount = 0;
        for (int x = 0; x < 8; ++x)
        {
            const auto& piece = grid[y][x];
            if (!piece)
            {
                emptyCount++;
            }
            else
            {
                if (emptyCount > 0)
                {
                    ss << emptyCount;
                    emptyCount = 0;
                }
                char c = piece->getType() == "knight" ? 'n' : piece->getType()[0];
                if (piece->getColor() == Color::White)
                    c = toupper(c);
                ss << c;
            }
        }
        if (emptyCount > 0)
            ss << emptyCount;
        if (y > 0)
            ss << "/";
    }

    // 2. Активный цвет
    ss << (current_player == Color::White ? " w " : " b ");

    // 3. Рокировка
    std::string castling = "";

    // Белые
    auto wk = grid[0][4].get(); // Король e1
    if (wk && wk->getType() == "king" && !wk->isMoved())
    {
        auto wr_k = grid[0][7].get(); // Ладья h1
        if (wr_k && wr_k->getType() == "rook" && !wr_k->isMoved())
            castling += "K";

        auto wr_q = grid[0][0].get(); // Ладья a1
        if (wr_q && wr_q->getType() == "rook" && !wr_q->isMoved())
            castling += "Q";
    }

    // Черные
    auto bk = grid[7][4].get(); // Король e8
    if (bk && bk->getType() == "king" && !bk->isMoved())
    {
        auto br_k = grid[7][7].get(); // Ладья h8
        if (br_k && br_k->getType() == "rook" && !br_k->isMoved())
            castling += "k";

        auto br_q = grid[7][0].get(); // Ладья a8
        if (br_q && br_q->getType() == "rook" && !br_q->isMoved())
            castling += "q";
    }

    ss << (castling.empty() ? "-" : castling) << " ";

    // 4. Взятие на проходе
    auto lastMoveOpt = getLastMove();
    std::string enPassant = "-";
    if (lastMoveOpt.has_value())
    {
        Move last = *lastMoveOpt;
        Position from = last.getFrom();
        Position to = last.getTo();

        // Проверяем, был ли ход пешкой на 2 клетки
        if (grid[to.getY()][to.getX()] && grid[to.getY()][to.getX()]->getType() == "pawn")
        {
            if (abs(from.getY() - to.getY()) == 2)
            {
                int epY = (from.getY() + to.getY()) / 2;
                char col = 'a' + from.getX();
                enPassant = col + std::to_string(epY + 1);
            }
        }
    }
    ss << enPassant;

    // 5. Полуходы и номер хода (упрощенно, так как Board не хранит счетчик 50 ходов)
    ss << " 0 " << (history.size() / 2 + 1);

    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Board& board)
{
    if (board.grid[0][0])
    {
        std::vector<Move> moves = board.grid[0][0]->getPossibleMoves(board.grid, board.getLastMove());
        for (auto& move : moves)
        {
            std::cout << move << '\n';
        }
    }
    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            if (board.grid[i][j])
            {
                std::string color = (board.grid[i][j]->getColor() == Color::White) ? "(w)" : "(b)";
                os << board.grid[i][j]->getType()[0] << color << '\t';
            }
            else
                os << "X" << '\t';
        }
        os << '\n';
    }
    return os;
}