#pragma once
#include "../core/pieses.h"
#include "../graphic/InputBox.h"
#include "../graphic/ResourceManager.h"
#include "../graphic/button.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <vector>
#include <ctime>

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
    std::string serverIp = "127.0.0.1";
};

enum class MenuScreen
{
    Main,
    GameSetup,
    Settings,
    About
};

enum class AnimType
{
    None,
    KingChase,
    PawnPromotion,
    KnightsLeap,
    BishopGlide
};

class MainMenu
{
    sf::RenderWindow& window;
    ResourceManager& resourceManager;

    MenuScreen currentScreen;
    GameConfig currentConfig;

    std::vector<std::unique_ptr<Button>> mainButtons;
    std::vector<std::unique_ptr<Button>> setupButtons;
    std::vector<std::unique_ptr<Button>> backButton;

    std::vector<std::unique_ptr<InputBox>> inputBoxes;
    std::vector<sf::Text> labels;

    std::unique_ptr<sf::Sprite> backgroundSprite;

    sf::Text errorText;
    sf::Text aboutText;

    std::function<void(GameConfig)> onStartGame;
    std::function<void()> onExit;

    // --- Анимация ---
    sf::Clock dtClock;
    float idleTimer;
    AnimType currentAnim;

    sf::Sprite animSprite1;
    sf::Sprite animSprite2;

    float animStateTime;
    float animPhase;
    bool animFlag;
    bool secondActorActive; // <-- Новый флаг для задержки ладьи

public:
    MainMenu(sf::RenderWindow& win, ResourceManager& rm);

    void handleEvent(const std::optional<sf::Event>& event);
    void update();
    void draw();
    void resize();

    void setOnStartGame(std::function<void(GameConfig)> callback) { onStartGame = callback; }
    void setOnExit(std::function<void()> callback) { onExit = callback; }

    void setErrorMessage(const std::string& message);

private:
    void initButtons();
    void createSetupButtons(sf::Vector2u winSize);

    void createLabel(float x, float y, const std::string& text, unsigned int size = 24);

    std::unique_ptr<Button> createBtn(
        sf::Vector2f pos, sf::Vector2f size, const std::string& text,
        std::function<void()> onClick, sf::Color color = sf::Color::White);

    void startRandomAnimation();
    void updateKingChase(float dt);
    void updatePawnPromotion(float dt);
    void updateKnightsLeap(float dt);
    void updateBishopGlide(float dt);
};