#include "game_mode.h"

std::unique_ptr<Piece> Test::clonePiece(const Piece& piece) const
{
    PieceFactory pf;
    pf.registration<Pawn>("pawn");
    pf.registration<Rook>("rook");
    pf.registration<Knight>("knight");
    pf.registration<Bishop>("bishop");
    pf.registration<Queen>("queen");
    pf.registration<King>("king");

    return pf.create(piece.getType(), piece.getColor(), piece.getPosition(), piece.isMoved());
}

void Test::initializeBoard(Piece::board_type& board)
{
    board.resize(8);
    for (auto& row : board)
    {
        row.resize(8);
    }

    PieceFactory pf;
    pf.registration<Pawn>("pawn");
    pf.registration<Rook>("rook");
    pf.registration<Knight>("knight");
    pf.registration<Bishop>("bishop");
    pf.registration<Queen>("queen");
    pf.registration<King>("king");
    for (int i = 0; i < 8; i++)
    {
        board[1][i] = pf.create("pawn", Color::White, Position(i, 1));
        board[6][i] = pf.create("pawn", Color::Black, Position(i, 6));
    }
    board[0][0] = pf.create("rook", Color::White, Position(0, 0));
    board[0][7] = pf.create("rook", Color::White, Position(7, 0));
    board[7][0] = pf.create("rook", Color::Black, Position(0, 7));
    board[7][7] = pf.create("rook", Color::Black, Position(7, 7));
    board[0][1] = pf.create("knight", Color::White, Position(1, 0));
    board[0][6] = pf.create("knight", Color::White, Position(6, 0));
    board[7][1] = pf.create("knight", Color::Black, Position(1, 7));
    board[7][6] = pf.create("knight", Color::Black, Position(6, 7));
    board[0][2] = pf.create("bishop", Color::White, Position(2, 0));
    board[0][5] = pf.create("bishop", Color::White, Position(5, 0));
    board[7][2] = pf.create("bishop", Color::Black, Position(2, 7));
    board[7][5] = pf.create("bishop", Color::Black, Position(5, 7));
    board[0][3] = pf.create("queen", Color::White, Position(3, 0));
    board[7][3] = pf.create("queen", Color::Black, Position(3, 7));
    board[0][4] = pf.create("king", Color::White, Position(4, 0));
    board[7][4] = pf.create("king", Color::Black, Position(4, 7));
}

void Test::move(Piece::board_type& board, Move move, std::optional<Color> currentPlayer)
{
    Position from = move.getFrom();
    Position to = move.getTo();

    board[to.getY()][to.getX()] = std::move(board[from.getY()][from.getX()]);
    board[to.getY()][to.getX()]->move(move);

    if (move.isCapture())
    {
        board[from.getY()][to.getX()] = nullptr;
    }
    else if (move.isCastling())
    {
        for (int dx = to.getX() > from.getX() ? 1 : -1; from.isValid(); from = Position(from.getX() + dx, from.getY()))
        {
            if (board[from.getY()][from.getX()] && board[from.getY()][from.getX()]->getType() == "rook")
            {
                board[to.getY()][to.getX() - dx] = std::move(board[from.getY()][from.getX()]);
                board[to.getY()][to.getX() - dx]->move(Move(Position(from.getX(), from.getY()), Position(to.getX() - dx, to.getY())));
                break;
            }
        }
    }
    else if (move.isPromotion())
    {
        if (!move.getPromotionPiece().empty() && currentPlayer.has_value())
        {
            PieceFactory pf;
            pf.registration<Queen>("queen");
            pf.registration<Rook>("rook");
            pf.registration<Bishop>("bishop");
            pf.registration<Knight>("knight");
            board[to.getY()][to.getX()] = pf.create(move.getPromotionPiece(), *currentPlayer, to);
        }
    }
    else
    {
        board[from.getY()][from.getX()] = nullptr;
    }
}

bool Test::isValidMove(const Piece::board_type& board, Color color, Move move, std::optional<Move> lastMove)
{
    if (move.isCastling())
    {
        if (isInCheck(board, color, lastMove))
            return false;

        Position kingPos = move.getFrom();
        Position kingTo = move.getTo();

        if (board[kingPos.getY()][kingPos.getX()]->isMoved())
            return false;

        std::vector<Move> enemyMoves = getAllMoves(board, color == Color::White ? Color::Black : Color::White, lastMove);
        int dx = kingTo.getX() > kingPos.getX() ? 1 : -1;
        kingPos = Position(kingPos.getX() + dx, kingPos.getY());
        for (; !(kingPos == kingTo); kingPos = Position(kingPos.getX() + dx, kingPos.getY()))
        {
            if (board[kingPos.getY()][kingPos.getX()]
                && (board[kingPos.getY()][kingPos.getX()]->getColor() != color
                    || board[kingPos.getY()][kingPos.getX()]->getType() != "rook"
                    || board[kingPos.getY()][kingPos.getX()]->isMoved()))
                return false;

            if (ceilInCheck(board, enemyMoves, kingPos))
                return false;
        }
    }

    auto tempGrid = copyBoard(board);
    this->move(tempGrid, move);
    if (isInCheck(tempGrid, color, move))
        return false;

    return true;
}

bool Test::isCheckmate(const Piece::board_type& board, Color color, std::optional<Move> lastMove)
{
    std::vector<Move> all_possible_moves = getAllMoves(board, color, lastMove);

    for (const Move& move : all_possible_moves)
    {
        if (!isValidMove(board, color, move, lastMove))
            continue;

        Piece::board_type board_copy = copyBoard(board);

        Position from = move.getFrom();
        Position to = move.getTo();

        board_copy[to.getY()][to.getX()] = std::move(board_copy[from.getY()][from.getX()]);
        board_copy[to.getY()][to.getX()]->move(move);

        if (!isInCheck(board_copy, color, move))
        {
            return false;
        }
    }

    return true;
}

Piece::board_type Test::copyBoard(const Piece::board_type& board) const
{
    Piece::board_type board_copy;
    board_copy.resize(8);

    for (int y = 0; y < 8; y++)
    {
        board_copy[y].resize(8);
        for (int x = 0; x < 8; x++)
        {
            if (board[y][x])
            {
                board_copy[y][x] = clonePiece(*board[y][x]);
            }
        }
    }

    return board_copy;
}

bool Test::ceilInCheck(const Piece::board_type& board, std::vector<Move> enemy_moves, Position pos) const
{
    return enemy_moves.end() != std::find_if(enemy_moves.begin(), enemy_moves.end(), [pos](const Move& move) {
        return move.getTo() == pos;
    });
}

bool Test::isInCheck(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const
{
    Position king_pos;
    for (const auto& row : board)
    {
        auto it = std::find_if(
            row.begin(),
            row.end(),
            [color](const auto& piece) {
                return piece && piece->getColor() == color && piece->getType() == "king";
            });
        if (it != row.end())
            king_pos = it->get()->getPosition();
    }
    if (!king_pos.isValid())
        return false;

    std::vector<Move> enemy_moves = getAllMoves(board, (color == Color::White) ? Color::Black : Color::White, lastMove);
    return ceilInCheck(board, enemy_moves, king_pos);
}

std::vector<Move> Test::getAllMoves(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const
{
    std::vector<Move> all_moves;
    for (const auto& row : board)
    {
        for (const auto& piece : row)
        {
            if (piece && piece->getColor() == color)
            {
                std::vector<Move> piece_moves = piece->getPossibleMoves(board, lastMove);
                all_moves.insert(all_moves.end(), piece_moves.begin(), piece_moves.end());
            }
        }
    }

    return all_moves;
}

bool Test::isStalemate(const Piece::board_type& board, Color color, std::optional<Move> lastMove)
{
    std::vector<Move> all_possible_moves = getAllMoves(board, color, lastMove);

    for (const Move& move : all_possible_moves)
    {
        if (!isValidMove(board, color, move, lastMove))
            continue;

        Piece::board_type board_copy = copyBoard(board);

        Position from = move.getFrom();
        Position to = move.getTo();

        board_copy[to.getY()][to.getX()] = std::move(board_copy[from.getY()][from.getX()]);
        board_copy[to.getY()][to.getX()]->move(move);

        if (!isInCheck(board_copy, color, lastMove))
        {
            return false;
        }
    }

    return true;
}

const std::vector<std::string>& Test::getPromotionTypes() const
{
    return promotionTypes;
}

std::unique_ptr<Piece> Fischer::clonePiece(const Piece& piece) const
{
    PieceFactory pf;
    pf.registration<Pawn>("pawn");
    pf.registration<Rook>("rook");
    pf.registration<Knight>("knight");
    pf.registration<Bishop>("bishop");
    pf.registration<Queen>("queen");
    pf.registration<King>("king");

    return pf.create(piece.getType(), piece.getColor(), piece.getPosition(), piece.isMoved());
}

void Fischer::initializeBoard(Piece::board_type& board)
{
    board.resize(8);
    for (auto& row : board)
    {
        row.resize(8);
    }

    PieceFactory pf;
    pf.registration<Pawn>("pawn");
    pf.registration<Rook>("rook");
    pf.registration<Knight>("knight");
    pf.registration<Bishop>("bishop");
    pf.registration<Queen>("queen");
    pf.registration<King>("king");
    srand(time(0));
    for (int i = 0; i < 8; i++)
    {
        board[1][i] = pf.create("pawn", Color::White, Position(i, 1));
        board[6][i] = pf.create("pawn", Color::Black, Position(i, 6));
    }
    std::vector<int> positions = { 0, 1, 2, 3, 4, 5, 6, 7 };
    int kingPosition = 1 + rand() % 6;
    int firstRookPosition = rand() % kingPosition;
    int secondRookPosition = kingPosition + 1 + (rand() % (7 - kingPosition));
    positions.erase(
        std::remove_if(positions.begin(), positions.end(), [kingPosition, firstRookPosition, secondRookPosition](int x) {
            return x == kingPosition || x == firstRookPosition || x == secondRookPosition;
            }),
        positions.end());
    int firstBishopPosition = positions[rand() % 5];
    int secondBishopPosition = positions[rand() % 5];
    while (secondBishopPosition % 2 == firstBishopPosition % 2)
        secondBishopPosition = positions[rand() % 5];
    positions.erase(
        std::remove_if(positions.begin(), positions.end(), [firstBishopPosition, secondBishopPosition](int x) {
            return x == firstBishopPosition || x == secondBishopPosition;
            }),
        positions.end());
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(positions.begin(), positions.end(), g);

    int queenPosition = positions[0];
    int firstKnightPosition = positions[1];
    int secondKnightPosition = positions[2];
    board[0][kingPosition] = pf.create("king", Color::White, Position(kingPosition, 0));
    board[0][firstRookPosition] = pf.create("rook", Color::White, Position(firstRookPosition, 0));
    board[0][secondRookPosition] = pf.create("rook", Color::White, Position(secondRookPosition, 0));
    board[0][queenPosition] = pf.create("queen", Color::White, Position(queenPosition, 0));
    board[0][firstBishopPosition] = pf.create("bishop", Color::White, Position(firstBishopPosition, 0));
    board[0][secondBishopPosition] = pf.create("bishop", Color::White, Position(secondBishopPosition, 0));
    board[0][firstKnightPosition] = pf.create("knight", Color::White, Position(firstKnightPosition, 0));
    board[0][secondKnightPosition] = pf.create("knight", Color::White, Position(secondKnightPosition, 0));
    board[7][kingPosition] = pf.create("king", Color::Black, Position(kingPosition, 7));
    board[7][firstRookPosition] = pf.create("rook", Color::Black, Position(firstRookPosition, 7));
    board[7][secondRookPosition] = pf.create("rook", Color::Black, Position(secondRookPosition, 7));
    board[7][queenPosition] = pf.create("queen", Color::Black, Position(queenPosition, 7));
    board[7][firstBishopPosition] = pf.create("bishop", Color::Black, Position(firstBishopPosition, 7));
    board[7][secondBishopPosition] = pf.create("bishop", Color::Black, Position(secondBishopPosition, 7));
    board[7][firstKnightPosition] = pf.create("knight", Color::Black, Position(firstKnightPosition, 7));
    board[7][secondKnightPosition] = pf.create("knight", Color::Black, Position(secondKnightPosition, 7));
}

void Fischer::move(Piece::board_type& board, Move move, std::optional<Color> currentPlayer)
{
    Position from = move.getFrom();
    Position to = move.getTo();

    board[to.getY()][to.getX()] = std::move(board[from.getY()][from.getX()]);
    board[to.getY()][to.getX()]->move(move);

    if (move.isCapture())
    {
        board[from.getY()][to.getX()] = nullptr;
    }
    else if (move.isCastling())
    {
        for (int dx = to.getX() > from.getX() ? 1 : -1; from.isValid(); from = Position(from.getX() + dx, from.getY()))
        {
            if (board[from.getY()][from.getX()] && board[from.getY()][from.getX()]->getType() == "rook")
            {
                board[to.getY()][to.getX() - dx] = std::move(board[from.getY()][from.getX()]);
                board[to.getY()][to.getX() - dx]->move(Move(Position(from.getX(), from.getY()), Position(to.getX() - dx, to.getY())));
                break;
            }
        }
    }
    else if (move.isPromotion())
    {
        if (!move.getPromotionPiece().empty() && currentPlayer.has_value())
        {
            PieceFactory pf;
            pf.registration<Queen>("queen");
            pf.registration<Rook>("rook");
            pf.registration<Bishop>("bishop");
            pf.registration<Knight>("knight");
            board[to.getY()][to.getX()] = pf.create(move.getPromotionPiece(), *currentPlayer, to);
        }
    }
    else
    {
        board[from.getY()][from.getX()] = nullptr;
    }
}

bool Fischer::isValidMove(const Piece::board_type& board, Color color, Move move, std::optional<Move> lastMove)
{
    if (move.isCastling())
    {
        if (isInCheck(board, color, lastMove))
            return false;

        Position kingPos = move.getFrom();
        Position kingTo = move.getTo();

        if (board[kingPos.getY()][kingPos.getX()]->isMoved())
            return false;

        std::vector<Move> enemyMoves = getAllMoves(board, color == Color::White ? Color::Black : Color::White, lastMove);
        int dx = kingTo.getX() > kingPos.getX() ? 1 : -1;
        kingPos = Position(kingPos.getX() + dx, kingPos.getY());
        for (; !(kingPos == kingTo); kingPos = Position(kingPos.getX() + dx, kingPos.getY()))
        {
            if (board[kingPos.getY()][kingPos.getX()]
                && (board[kingPos.getY()][kingPos.getX()]->getColor() != color
                    || board[kingPos.getY()][kingPos.getX()]->getType() != "rook"
                    || board[kingPos.getY()][kingPos.getX()]->isMoved()))
                return false;

            if (ceilInCheck(board, enemyMoves, kingPos))
                return false;
        }
    }

    auto tempGrid = copyBoard(board);
    this->move(tempGrid, move);
    if (isInCheck(tempGrid, color, move))
        return false;

    return true;
}

bool Fischer::isCheckmate(const Piece::board_type& board, Color color, std::optional<Move> lastMove)
{
    if (!isInCheck(board, color, lastMove))
        return false;

    std::vector<Move> all_possible_moves = getAllMoves(board, color, lastMove);

    for (const Move& move : all_possible_moves)
    {
        if (isValidMove(board, color, move, lastMove))
        {
            return false;
        }
    }

    return true;
}

Piece::board_type Fischer::copyBoard(const Piece::board_type& board) const
{
    Piece::board_type board_copy;
    board_copy.resize(8);

    for (int y = 0; y < 8; y++)
    {
        board_copy[y].resize(8);
        for (int x = 0; x < 8; x++)
        {
            if (board[y][x])
            {
                board_copy[y][x] = clonePiece(*board[y][x]);
            }
        }
    }

    return board_copy;
}

bool Fischer::ceilInCheck(const Piece::board_type& board, std::vector<Move> enemy_moves, Position pos) const
{
    return enemy_moves.end() != std::find_if(enemy_moves.begin(), enemy_moves.end(), [pos](const Move& move) {
        return move.getTo() == pos;
        });
}

bool Fischer::isInCheck(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const
{
    Position king_pos;
    for (const auto& row : board)
    {
        auto it = std::find_if(
            row.begin(),
            row.end(),
            [color](const auto& piece) {
                return piece && piece->getColor() == color && piece->getType() == "king";
            });
        if (it != row.end())
            king_pos = it->get()->getPosition();
    }

    std::vector<Move> enemy_moves = getAllMoves(board, (color == Color::White) ? Color::Black : Color::White, lastMove);
    return ceilInCheck(board, enemy_moves, king_pos);
}

std::vector<Move> Fischer::getAllMoves(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const
{
    std::vector<Move> all_moves;
    for (const auto& row : board)
    {
        for (const auto& piece : row)
        {
            if (piece && piece->getColor() == color)
            {
                std::vector<Move> piece_moves = piece->getPossibleMoves(board, lastMove);
                all_moves.insert(all_moves.end(), piece_moves.begin(), piece_moves.end());
            }
        }
    }

    return all_moves;
}

bool Fischer::isStalemate(const Piece::board_type& board, Color color, std::optional<Move> lastMove) const
{
    if (isInCheck(board, color, lastMove))
        return false;

    auto* nonConstThis = const_cast<Fischer*>(this);
    std::vector<Move> all_possible_moves = getAllMoves(board, color, lastMove);

    for (const Move& move : all_possible_moves)
    {
        if (nonConstThis->isValidMove(board, color, move, lastMove))
        {
            return false;
        }
    }

    return true;
}

const std::vector<std::string>& Fischer::getPromotionTypes() const
{
    return promotionTypes;
}