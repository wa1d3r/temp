#pragma once
#include "../core/pieses.h"
#include "../graphic/ResourceManager.h"
#include "../graphic/button.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <vector>

enum class AppState
{
    Menu,
    Game,
    Exit,
    EndGame
};

enum class GameType
{
    Classic,
    Fischer
};

enum class OpponentType
{
    Local,
    Network,
    AI
};

struct GameConfig
{
    GameType gameType = GameType::Classic;
    OpponentType opponentType = OpponentType::Local;
    Color playerColor = Color::White;
    float timeMinutes = 10.0f;
    float incrementSeconds = 5.0f;
};

enum class MenuScreen
{
    Main,
    GameSetup,
    Settings,
    About
};

class MainMenu
{
    sf::RenderWindow& window;
    ResourceManager& resourceManager;

    MenuScreen currentScreen;
    GameConfig currentConfig;

    std::vector<std::unique_ptr<Button>> mainButtons;
    std::vector<std::unique_ptr<Button>> setupButtons;
    std::vector<std::unique_ptr<Button>> aboutButtons;
    std::vector<std::unique_ptr<Button>> backButton;

    std::unique_ptr<sf::Sprite> backgroundSprite;

    std::function<void(GameConfig)> onStartGame;
    std::function<void()> onExit;

public:
    MainMenu(sf::RenderWindow& win, ResourceManager& rm);

    void handleEvent(const std::optional<sf::Event>& event);
    void draw();
    void resize();

    void setOnStartGame(std::function<void(GameConfig)> callback) { onStartGame = callback; }
    void setOnExit(std::function<void()> callback) { onExit = callback; }

private:
    void initButtons();
    void createSetupButtons(sf::Vector2u winSize);
    std::unique_ptr<Button> createBtn(
        sf::Vector2f pos, sf::Vector2f size, const std::string& text,
        std::function<void()> onClick, sf::Color color = sf::Color::White);
};