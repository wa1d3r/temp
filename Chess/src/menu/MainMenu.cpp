#include "MainMenu.h"
#include <iomanip>
#include <iostream>
#include <sstream>

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

// Хелпер создания меток
void MainMenu::createLabel(float x, float y, const std::string& str, unsigned int size)
{
    sf::Text text(*resourceManager.getFont("main_font"), str);
    text.setCharacterSize(size);
    text.setFillColor(sf::Color(200, 200, 200)); // Светло-серый

    // Позиционируем по координатам экрана (проценты -> пиксели)
    sf::FloatRect bounds = text.getLocalBounds();
    text.setPosition(sf::Vector2f(window.getSize().x * x, window.getSize().y * y));

    labels.push_back(text);
}

std::unique_ptr<Button> MainMenu::createBtn(sf::Vector2f pos, sf::Vector2f size,
    const std::string& text, std::function<void()> onClick, sf::Color color)
{
    const sf::Font* font = resourceManager.getFont("main_font");
    return std::make_unique<Button>(
        pos, size, window.getSize(), 0.005f,
        onClick, text, 24, font,
        color, sf::Color(200, 200, 255), sf::Color::Black, sf::Color::Black);
}

void MainMenu::initButtons()
{
    sf::Vector2u winSize = window.getSize();
    mainButtons.clear();
    setupButtons.clear();
    aboutButtons.clear();
    backButton.clear();
    inputBoxes.clear();
    labels.clear(); // Очищаем метки

    float btnW = 0.3f;
    float btnH = 0.08f;
    float startY = 0.3f;
    float gap = 0.12f;

    // --- MAIN SCREEN ---
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

    // --- SETUP SCREEN ---
    backButton.push_back(createBtn({ 0.05f, 0.9f }, { 0.15f, 0.06f }, "< Back", [this]() {
        currentScreen = MenuScreen::Main;
        initButtons();
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
    float row = 0.15f;

    // 1. OPPONENT
    setupButtons.push_back(createBtn({ col1, row }, { 0.25f, 0.08f }, "Local (2P)", [this]() {
        currentConfig.opponentType = OpponentType::Local;
        initButtons(); }, currentConfig.opponentType == OpponentType::Local ? sf::Color::Green : sf::Color::White));

    setupButtons.push_back(createBtn({ col2, row }, { 0.25f, 0.08f }, "Network", [this]() {
        currentConfig.opponentType = OpponentType::Network;
        initButtons(); }, currentConfig.opponentType == OpponentType::Network ? sf::Color::Green : sf::Color::White));

    setupButtons.push_back(createBtn({ col3, row }, { 0.25f, 0.08f }, "Stockfish AI", [this]() {
        currentConfig.opponentType = OpponentType::AI;
        initButtons(); }, currentConfig.opponentType == OpponentType::AI ? sf::Color::Green : sf::Color::White));

    row += 0.15f;

    // 2. COLOR
    setupButtons.push_back(createBtn({ col1, row }, { 0.25f, 0.08f }, "White", [this]() {
        currentConfig.playerColor = Color::White;
        currentConfig.isRandomColor = false;
        initButtons(); }, (!currentConfig.isRandomColor && currentConfig.playerColor == Color::White) ? sf::Color::Green : sf::Color::White));

    setupButtons.push_back(createBtn({ col2, row }, { 0.25f, 0.08f }, "Random", [this]() {
        currentConfig.isRandomColor = true;
        initButtons(); }, currentConfig.isRandomColor ? sf::Color::Green : sf::Color::White));

    setupButtons.push_back(createBtn({ col3, row }, { 0.25f, 0.08f }, "Black", [this]() {
        currentConfig.playerColor = Color::Black;
        currentConfig.isRandomColor = false;
        initButtons(); }, (!currentConfig.isRandomColor && currentConfig.playerColor == Color::Black) ? sf::Color::Green : sf::Color::White));

    row += 0.15f;

    // 3. CUSTOM TIME INPUTS (Создаем до пресетов, чтобы пресеты могли их обновлять)
    const sf::Font* font = resourceManager.getFont("main_font");
    float customRow = row + 0.16f; // Сдвигаем ниже пресетов
    float labelYOffset = 0.01f; // Подстройка по вертикали для текста

    // Ширины элементов
    float labelW = 0.15f;
    float inputW = 0.10f;

    // Координаты для Min
    float minLabelX = 0.15f;
    float minInputX = minLabelX + labelW; // Сразу после надписи

    // Координаты для Inc
    float incLabelX = 0.55f;
    float incInputX = incLabelX + labelW;

    // Создаем InputBox
    auto minInput = std::make_unique<InputBox>(
        sf::Vector2f(winSize.x * minInputX, winSize.y * customRow),
        sf::Vector2f(winSize.x * inputW, winSize.y * 0.06f),
        *font, std::to_string((int)currentConfig.timeMinutes));

    auto incInput = std::make_unique<InputBox>(
        sf::Vector2f(winSize.x * incInputX, winSize.y * customRow),
        sf::Vector2f(winSize.x * inputW, winSize.y * 0.06f),
        *font, std::to_string((int)currentConfig.incrementSeconds));

    InputBox* pMinInput = minInput.get();
    InputBox* pIncInput = incInput.get();

    // Создаем надписи (Labels)
    createLabel(minLabelX, customRow + labelYOffset, "Custom Min:");
    createLabel(incLabelX, customRow + labelYOffset, "Inc (sec):");

    // Обработчики ввода
    pMinInput->setOnChange([this](std::string val) {
        try
        {
            currentConfig.timeMinutes = std::stof(val);
        }
        catch (...)
        {
        }
    });
    pIncInput->setOnChange([this](std::string val) {
        try
        {
            currentConfig.incrementSeconds = std::stof(val);
        }
        catch (...)
        {
        }
    });

    // 4. TIME PRESETS
    auto timeBtn = [&](std::string txt, float m, float inc, float x, float y) {
        bool selected = (std::abs(currentConfig.timeMinutes - m) < 0.01f && std::abs(currentConfig.incrementSeconds - inc) < 0.01f);

        setupButtons.push_back(createBtn({ x, y }, { 0.15f, 0.06f }, txt, [this, m, inc, pMinInput, pIncInput]() {
                currentConfig.timeMinutes = m;
                currentConfig.incrementSeconds = inc;
                std::stringstream ssMin, ssInc;
                ssMin << m; ssInc << inc;
                pMinInput->setString(ssMin.str());
                pIncInput->setString(ssInc.str());
                initButtons(); }, selected ? sf::Color::Green : sf::Color::White));
    };

    // Ряд 1
    timeBtn("1 + 0", 1, 0, 0.1f, row);
    timeBtn("3 + 0", 3, 0, 0.27f, row);
    timeBtn("3 + 2", 3, 2, 0.44f, row);
    timeBtn("5 + 0", 5, 0, 0.61f, row);
    timeBtn("5 + 3", 5, 3, 0.78f, row);

    row += 0.08f;
    // Ряд 2
    timeBtn("10 + 0", 10, 0, 0.1f, row);
    timeBtn("10 + 5", 10, 5, 0.27f, row);
    timeBtn("15 + 10", 15, 10, 0.44f, row);
    timeBtn("30 + 0", 30, 0, 0.61f, row);
    timeBtn("30 + 20", 30, 20, 0.78f, row);

    // Добавляем инпуты в общий список
    inputBoxes.push_back(std::move(minInput));
    inputBoxes.push_back(std::move(incInput));

    // START BUTTON
    setupButtons.push_back(createBtn({ 0.35f, 0.85f }, { 0.3f, 0.1f }, "START GAME", [this]() {
        if (onStartGame) {
            if (currentConfig.isRandomColor) {
                currentConfig.playerColor = (rand() % 2 == 0) ? Color::White : Color::Black;
            }
            if (currentConfig.timeMinutes <= 0) currentConfig.timeMinutes = 1;
            onStartGame(currentConfig);
        } }, sf::Color(100, 255, 100)));
}

void MainMenu::handleEvent(const std::optional<sf::Event>& event)
{
    if (currentScreen == MenuScreen::Main)
    {
        for (auto& btn : mainButtons)
            btn->handleEvent(event, window);
    }
    else if (currentScreen == MenuScreen::GameSetup)
    {
        for (auto& btn : setupButtons)
            btn->handleEvent(event, window);
        for (auto& btn : backButton)
            btn->handleEvent(event, window);
        for (auto& box : inputBoxes)
            box->handleEvent(event, window);
    }
    else if (currentScreen == MenuScreen::About)
    {
        for (auto& btn : aboutButtons)
            btn->handleEvent(event, window);
        for (auto& btn : backButton)
            btn->handleEvent(event, window);
    }
    else if (currentScreen == MenuScreen::Settings)
    {
        for (auto& btn : backButton)
            btn->handleEvent(event, window);
    }
}

void MainMenu::draw()
{
    window.draw(*backgroundSprite);

    if (currentScreen == MenuScreen::Main)
    {
        for (auto& btn : mainButtons)
            btn->draw(window);
    }
    else if (currentScreen == MenuScreen::GameSetup)
    {
        for (auto& btn : setupButtons)
            btn->draw(window);
        for (auto& btn : backButton)
            btn->draw(window);
        for (auto& box : inputBoxes)
            box->draw(window);
        // Рисуем метки
        for (auto& label : labels)
            window.draw(label);
    }
    else if (currentScreen == MenuScreen::About)
    {
        for (auto& btn : aboutButtons)
            btn->draw(window);
        for (auto& btn : backButton)
            btn->draw(window);
    }
    else if (currentScreen == MenuScreen::Settings)
    {
        for (auto& btn : backButton)
            btn->draw(window);
    }
}