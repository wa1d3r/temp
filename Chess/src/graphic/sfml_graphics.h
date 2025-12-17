#pragma once
#include "../game_interfaces.h"
#include "ResourceManager.h"
#include "button.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <iomanip>
#include <sstream>
#include <vector>

class SFMLGraphics : public IGraphicsInterface
{
    sf::RenderWindow& window;
    ResourceManager& resourceManager;
    std::vector<std::unique_ptr<Button>> boardButtons;
    std::vector<std::unique_ptr<sf::RenderTexture>> cellBuffers;
    std::vector<Highlight> highlighted;
    std::vector<std::unique_ptr<Button>> promotionButtons;
    bool isPromotionActive = false;
    sf::RectangleShape promotionBgShape;
    std::unique_ptr<sf::Sprite> backgroundSprite;

    sf::FloatRect boardArea;
    sf::FloatRect topClockArea;
    sf::FloatRect bottomClockArea;

    std::string currentMessage;
    bool isMessageVisible = false;

    sf::FloatRect historyArea;
    std::unique_ptr<Button> resignButton;
    std::function<void()> onResignCallback;

    Color viewColor;

    std::function<void(Position)> onSquareClickCallback;

    Position getChangedPos(Position pos)
    {
        if (Color::White == viewColor)
        {
            return Position(pos.getX(), 7 - pos.getY());
        }
        else
        {
            return Position(7 - pos.getX(), pos.getY());
        }
    }

public:
    SFMLGraphics(sf::RenderWindow& win, ResourceManager& rm, Color playerColor)
        : window(win)
        , resourceManager(rm)
        , viewColor(playerColor)
    {
        cellBuffers.resize(64);
        highlighted.resize(64, Highlight::NO_HIGHLIGHT);
        for (auto& buf : cellBuffers)
        {
            buf = std::make_unique<sf::RenderTexture>();
        }
        createBoardButtons();
    }

    void createBoardButtons()
    {
        boardButtons.clear();
        sf::Vector2u winSize = window.getSize();

        // ... (код загрузки фона и расчета масштаба остается тем же) ...
        const sf::Texture* bgTex = resourceManager.getTexture("background");
        backgroundSprite = std::make_unique<sf::Sprite>(*bgTex);
        float scaleX = static_cast<float>(winSize.x) / bgTex->getSize().x;
        float scaleY = static_cast<float>(winSize.y) / bgTex->getSize().y;
        backgroundSprite->setScale(sf::Vector2f(scaleX, scaleY));
        backgroundSprite->setPosition(sf::Vector2f(0.f, 0.f));

        float winW = static_cast<float>(winSize.x);
        float winH = static_cast<float>(winSize.y);

        // Параметры верстки
        float marginRatio = 0.05f;
        float clockWidthRatio = 0.25f;
        float clockGapRatio = 0.05f;

        float maxSideByHeight = winH * (1.0f - 2.0f * marginRatio);
        float availableWidthForContent = winW * (1.0f - 2.0f * marginRatio);
        float widthFactor = 1.0f + clockGapRatio + clockWidthRatio;
        float maxSideByWidth = availableWidthForContent / widthFactor;
        float boardSide = min(maxSideByHeight, maxSideByWidth);

        float cellSizePx = boardSide / 8.0f;
        float clockWidth = boardSide * clockWidthRatio;

        // Высоты элементов правой панели
        float clockHeight = boardSide * 0.15f;
        float buttonHeight = boardSide * 0.10f;
        float spacing = boardSide * 0.02f; // Отступы между элементами

        // Остаток места под историю: Общая высота - 2 часов - кнопка - отступы
        float historyHeight = boardSide - (2 * clockHeight) - buttonHeight - (3 * spacing);

        // Координаты
        float totalGroupWidth = boardSide + (boardSide * clockGapRatio) + clockWidth;
        float startX = (winW - totalGroupWidth) / 2.0f;
        float startY = (winH - boardSide) / 2.0f;

        boardArea = sf::FloatRect(sf::Vector2f(startX, startY), sf::Vector2f(boardSide, boardSide));
        float rightPanelX = startX + boardSide + (boardSide * clockGapRatio);

        // 1. Верхние часы (Черные)
        topClockArea = sf::FloatRect(sf::Vector2f(rightPanelX, startY), sf::Vector2f(clockWidth, clockHeight));

        // 2. История ходов (Между часами)
        float historyY = startY + clockHeight + spacing;
        historyArea = sf::FloatRect(sf::Vector2f(rightPanelX, historyY), sf::Vector2f(clockWidth, historyHeight));

        // 3. Нижние часы (Белые)
        float bottomClockY = historyY + historyHeight + spacing;
        bottomClockArea = sf::FloatRect(sf::Vector2f(rightPanelX, bottomClockY), sf::Vector2f(clockWidth, clockHeight));

        // 4. Кнопка "Сдаться" (В самом низу)
        float buttonY = bottomClockY + clockHeight + spacing;

        sf::Vector2f btnRelPos(rightPanelX / winW, buttonY / winH);
        sf::Vector2f btnRelSize(clockWidth / winW, buttonHeight / winH);

        resignButton = std::make_unique<Button>(
            btnRelPos, btnRelSize, winSize, 0.f,
            [this]() { if (onResignCallback) onResignCallback(); },
            "Resign", static_cast<unsigned int>(buttonHeight * 0.4f),
            resourceManager.getFont("main_font"),
            sf::Color(200, 100, 100), sf::Color(255, 100, 100), // Красные оттенки
            sf::Color::White, sf::Color::Black);

        // ... (код создания кнопок доски остается тем же) ...
        auto texSize = resourceManager.getTexture("pawn_white")->getSize();
        for (auto& buf : cellBuffers)
        {
            buf->resize(sf::Vector2u(texSize.x, texSize.y));
        }
        for (int y = 0; y < 8; y++)
        {
            for (int x = 0; x < 8; x++)
            {
                sf::Color color = sf::Color::Transparent;
                sf::Color hover = sf::Color::Transparent;
                Position buttonPosition = getChangedPos(Position(x, y));

                float btnPxX = startX + buttonPosition.getX() * cellSizePx;
                float btnPxY = startY + buttonPosition.getY() * cellSizePx;

                sf::Vector2f relPos(btnPxX / winW, btnPxY / winH);
                sf::Vector2f relSize(cellSizePx / winW, cellSizePx / winH);

                auto btn = std::make_unique<Button>(
                    relPos,
                    relSize,
                    winSize,
                    0,
                    [this, x, y]() {
                        if (onSquareClickCallback)
                            onSquareClickCallback(Position(x, y));
                    },
                    "", 0, nullptr,
                    color, hover, sf::Color::Transparent, sf::Color::Transparent);

                boardButtons.push_back(std::move(btn));
            }
        }
    }

    void setOnClickCallback(std::function<void(Position)> callback)
    {
        onSquareClickCallback = callback;
    }

    void setOnResign(std::function<void()> callback) override
    {
        onResignCallback = callback;
    }

    void handleEvent(const std::optional<sf::Event>& event)
    {
        if (isPromotionActive)
        {
            for (auto& btn : promotionButtons)
            {
                btn->handleEvent(event, window);
            }
            return;
        }

        resignButton->handleEvent(event, window);

        for (auto& btn : boardButtons)
        {
            btn->handleEvent(event, window);
        }
    }

    void drawBoard(const Board& board, Color currentPlayer) override
    {
        auto& grid = board.getGrid();
        window.draw(*backgroundSprite);

        for (int y = 0; y < 8; y++)
        {
            for (int x = 0; x < 8; x++)
            {
                int index = y * 8 + x;
                Button* btn = boardButtons[index].get();
                auto& buffer = *cellBuffers[index];
                buffer.clear();

                std::string bgKey = ((x + y + 1) % 2 == 0) ? "cell_white" : "cell_black";
                sf::Sprite bgSprite(*resourceManager.getTexture(bgKey));

                float scaleX = static_cast<float>(buffer.getSize().x) / bgSprite.getTexture().getSize().x;
                float scaleY = static_cast<float>(buffer.getSize().y) / bgSprite.getTexture().getSize().y;
                bgSprite.setScale(sf::Vector2f(scaleX, scaleY));
                buffer.draw(bgSprite);

                if (highlighted[index] != Highlight::NO_HIGHLIGHT)
                {
                    std::string hlKey = (Highlight::CURRENT_POS == highlighted[index]) ? "current_pos"
                        : (Highlight::LAST_POS == highlighted[index])                  ? "last_pos"
                        : (Highlight::CHECK_POS == highlighted[index])                 ? "check"
                        : (Highlight::FRAME == highlighted[index])                     ? "frame"
                        : (Highlight::POINT == highlighted[index] && grid[y][x])       ? "frame"
                                                                                       : "point";
                    sf::Sprite hlSprite(*resourceManager.getTexture(hlKey));
                    float scaleX = static_cast<float>(buffer.getSize().x) / hlSprite.getTexture().getSize().x;
                    float scaleY = static_cast<float>(buffer.getSize().y) / hlSprite.getTexture().getSize().y;
                    hlSprite.setScale(sf::Vector2f(scaleX, scaleY));
                    buffer.draw(hlSprite);
                }

                if (grid[y][x])
                {
                    const Piece& p = *grid[y][x];
                    std::string key = p.getType() + "_" + (p.getColor() == Color::White ? "white" : "black");
                    auto pieceTexture = resourceManager.getTexture(key);

                    sf::Sprite pieceSprite(*pieceTexture);
                    float scaleX = static_cast<float>(buffer.getSize().x) / pieceTexture->getSize().x;
                    float scaleY = static_cast<float>(buffer.getSize().y) / pieceTexture->getSize().y;
                    pieceSprite.setScale(sf::Vector2f(scaleX, scaleY));
                    buffer.draw(pieceSprite);
                }

                buffer.display();

                btn->setTexture(&buffer.getTexture());
                btn->draw(window);
            }
        }

        float whiteTime = board.getWhiteTime();
        float blackTime = board.getBlackTime();

        bool isTopActive = currentPlayer != viewColor;
        drawClock(blackTime, topClockArea, resourceManager.getTexture(isTopActive ? "active_clock" : "disactive_clock"));
        drawClock(whiteTime, bottomClockArea, resourceManager.getTexture(isTopActive ? "disactive_clock" : "active_clock"));

        if (isPromotionActive)
        {
            window.draw(promotionBgShape);

            for (auto& btn : promotionButtons)
            {
                btn->draw(window);
            }
        }

        drawHistoryList(board.getHistory());

        resignButton->draw(window);

        if (isMessageVisible)
        {
            drawMessageOverlay();
        }
    }

    void highlightMoves(const std::vector<Move>& moves) override
    {
        for (const auto& move : moves)
        {
            Position to = move.getTo();
            setCellTypeHl(to, Highlight::POINT);
        }
    }

    void setCellTypeHl(Position pos, Highlight hl) override
    {
        int index = pos.getY() * 8 + pos.getX();
        highlighted[index] = hl;
    }

    void clearHighlights() override
    {
        std::fill(highlighted.begin(), highlighted.end(), Highlight::NO_HIGHLIGHT);
    }

    void showPromotionSelector(Color color, std::function<void(std::string)> callback, const std::vector<std::string>& promotionTypes) override
    {
        isPromotionActive = true;
        promotionButtons.clear();

        sf::Vector2u winSize = window.getSize();
        float minSide = min(static_cast<float>(winSize.x), static_cast<float>(winSize.y));
        float margin = minSide * 0.05f;
        float boardSide = minSide - 2.0f * margin;
        float cellSize = (minSide * 0.9f) / 8.0f;

        float startX = margin + 2 * cellSize;
        float startY = margin + 4 * cellSize;

        std::string suffix = (color == Color::White) ? "_white" : "_black";

        promotionBgShape.setSize(sf::Vector2f(8.0f * cellSize, 8.0f * cellSize));
        promotionBgShape.setPosition(sf::Vector2f(margin, margin));

        promotionBgShape.setFillColor(sf::Color(50, 50, 50, 200));

        int i = 0;
        for (const auto& type : promotionTypes)
        {
            float btnX = startX + (i++ * cellSize);
            sf::Vector2f relPos(btnX / winSize.x, startY / winSize.y);
            sf::Vector2f relSize(cellSize / winSize.x, cellSize / winSize.y);

            auto btn = std::make_unique<Button>(
                relPos, relSize, winSize, 0,
                [callback, type]() { callback(type); },
                "", 0, nullptr,
                sf::Color(240, 240, 240, 200),
                sf::Color(200, 200, 255, 255),
                sf::Color::Black, sf::Color::Black,
                resourceManager.getTexture(type + suffix));
            promotionButtons.push_back(std::move(btn));
        }
    }

    void hidePromotionSelector() override
    {
        isPromotionActive = false;
    }
};