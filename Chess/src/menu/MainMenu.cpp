#include "MainMenu.h"
#include <iostream>

MainMenu::MainMenu(sf::RenderWindow& win, ResourceManager& rm)
    : window(win)
    , resourceManager(rm)
    , currentScreen(MenuScreen::Main)
{
    backgroundSprite = std::make_unique<sf::Sprite>(*resourceManager.getTexture("background"));
    resize();
}

void MainMenu::resize()
{
    const sf::Texture bgTex = backgroundSprite->getTexture();
    float scaleX = static_cast<float>(window.getSize().x) / bgTex.getSize().x;
    float scaleY = static_cast<float>(window.getSize().y) / bgTex.getSize().y;
    backgroundSprite->setScale(sf::Vector2f(scaleX, scaleY));
    initButtons();
}

std::unique_ptr<Button> MainMenu::createBtn(sf::Vector2f pos, sf::Vector2f size,
    const std::string& text, std::function<void()> onClick, sf::Color color)
{
    const sf::Font* font = resourceManager.getFont("main_font");

    return std::make_unique<Button>(
        pos, size, window.getSize(), 0.005f,
        onClick, text, 30, font,
        color, sf::Color(200, 200, 255), sf::Color::Black, sf::Color::Black);
}

void MainMenu::initButtons()
{
    sf::Vector2u winSize = window.getSize();
    mainButtons.clear();
    setupButtons.clear();
    aboutButtons.clear();
    backButton.clear();

    float btnW = 0.3f;
    float btnH = 0.08f;
    float startY = 0.3f;
    float gap = 0.12f;

    mainButtons.push_back(createBtn({ 0.35f, startY }, { btnW, btnH }, "Classic Chess", [this]() {
        currentConfig.gameType = GameType::Classic;
        currentScreen = MenuScreen::GameSetup;
        initButtons();
    }));

    mainButtons.push_back(createBtn({ 0.35f, startY + gap }, { btnW, btnH }, "Fischer Chess", [this]() {
        currentConfig.gameType = GameType::Fischer;
        currentScreen = MenuScreen::GameSetup;
        initButtons(); }, sf::Color(200, 200, 200)));

    mainButtons.push_back(createBtn({ 0.35f, startY + gap * 2 }, { btnW, btnH }, "Settings", [this]() {
        currentScreen = MenuScreen::Settings;
    }));

    mainButtons.push_back(createBtn({ 0.35f, startY + gap * 3 }, { btnW, btnH }, "About", [this]() {
        currentScreen = MenuScreen::About;
    }));

    mainButtons.push_back(createBtn({ 0.35f, startY + gap * 4 }, { btnW, btnH }, "Exit", [this]() {
        if (onExit) onExit(); }, sf::Color(255, 150, 150)));

    backButton.push_back(createBtn({ 0.05f, 0.9f }, { 0.15f, 0.06f }, "< Back", [this]() {
        currentScreen = MenuScreen::Main;
    }));

    if (currentScreen == MenuScreen::GameSetup)
    {
        createSetupButtons(winSize);
    }

    if (currentScreen == MenuScreen::About)
    {
        aboutButtons.push_back(createBtn({ 0.2f, 0.3f }, { 0.6f, 0.4f }, "Chess Game v1.0\nDevs: You\n\nBuilt with SFML", []() {}, sf::Color(240, 240, 240)));
    }
}

void MainMenu::createSetupButtons(sf::Vector2u winSize)
{
    float col1 = 0.1f, col2 = 0.4f, col3 = 0.7f;
    float row = 0.2f;

    setupButtons.push_back(createBtn({ col1, row }, { 0.25f, 0.08f }, "Local (2P)", [this]() {
        currentConfig.opponentType = OpponentType::Local;
        initButtons();
    },
        currentConfig.opponentType == OpponentType::Local ? sf::Color::Green : sf::Color::White));

    setupButtons.push_back(createBtn({ col2, row }, { 0.25f, 0.08f }, "Network", [this]() {
        currentConfig.opponentType = OpponentType::Network;
        initButtons(); }, currentConfig.opponentType == OpponentType::Network ? sf::Color::Green : sf::Color::White));

    setupButtons.push_back(createBtn({ col3, row }, { 0.25f, 0.08f }, "Stockfish AI", [this]() {
        currentConfig.opponentType = OpponentType::AI;
        initButtons(); }, currentConfig.opponentType == OpponentType::AI ? sf::Color::Green : sf::Color::White));

    row += 0.15f;

    setupButtons.push_back(createBtn({ col1, row }, { 0.25f, 0.08f }, "White", [this]() {
        currentConfig.playerColor = Color::White;
        initButtons(); }, currentConfig.playerColor == Color::White ? sf::Color::Green : sf::Color::White));

    setupButtons.push_back(createBtn({ col2, row }, { 0.25f, 0.08f }, "Black", [this]() {
        currentConfig.playerColor = Color::Black;
        initButtons(); }, currentConfig.playerColor == Color::Black ? sf::Color::Green : sf::Color::White));

    row += 0.15f;

    auto timeBtn = [&](std::string txt, float m, float inc, float x) {
        bool selected = (currentConfig.timeMinutes == m && currentConfig.incrementSeconds == inc);
        setupButtons.push_back(createBtn({ x, row }, { 0.2f, 0.08f }, txt, [this, m, inc]() {
            currentConfig.timeMinutes = m;
            currentConfig.incrementSeconds = inc;
            initButtons(); }, selected ? sf::Color::Green : sf::Color::White));
    };

    timeBtn("1 + 0", 1, 0, col1);
    timeBtn("3 + 2", 3, 2, col2);
    timeBtn("10 + 5", 10, 5, col3);

    setupButtons.push_back(createBtn({ 0.35f, 0.8f }, { 0.3f, 0.1f }, "START GAME", [this]() {
        if (onStartGame) onStartGame(currentConfig); }, sf::Color(100, 255, 100)));
}