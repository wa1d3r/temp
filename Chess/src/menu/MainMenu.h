#pragma once
#include "../core/pieses.h"
#include "../graphic/InputBox.h"
#include "../graphic/ResourceManager.h"
#include "../graphic/button.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <vector>

// ... (Остальные enum и struct без изменений) ...
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
    bool isRandomColor = false;
    float timeMinutes = 10.0f;
    float incrementSeconds = 5.0f;
    int seed = 0;
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

    std::vector<std::unique_ptr<InputBox>> inputBoxes;
    std::vector<sf::Text> labels; // <--- НОВОЕ: Хранилище для простых надписей

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

    // Хелпер для создания надписей
    void createLabel(float x, float y, const std::string& text, unsigned int size = 24);

    std::unique_ptr<Button> createBtn(
        sf::Vector2f pos, sf::Vector2f size, const std::string& text,
        std::function<void()> onClick, sf::Color color = sf::Color::White);
};