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

    void setOnClickCallback(std::function<void(Position)> callback)
    {
        onSquareClickCallback = callback;
    }
};