#include "MainMenu.h"
#include <iomanip>
#include <iostream>
#include <sstream>

// Инициализируем errorText шрифтом прямо в списке инициализации
MainMenu::MainMenu(sf::RenderWindow& win, ResourceManager& rm)
    : window(win)
    , resourceManager(rm)
    , currentScreen(MenuScreen::Main)
    , errorText(*rm.getFont("main_font")) // <-- Исправление ошибки конструктора
{
    backgroundSprite = std::make_unique<sf::Sprite>(*resourceManager.getTexture("background"));

    // Настраиваем стиль текста ошибки
    errorText.setCharacterSize(30); // Чуть крупнее для заметности
    errorText.setFillColor(sf::Color::Red);
    // Обводка для читаемости на любом фоне
    errorText.setOutlineColor(sf::Color::Black);
    errorText.setOutlineThickness(2.0f);

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

void MainMenu::setErrorMessage(const std::string& message)
{
    errorText.setString(message);

    // Центрируем точку привязки текста
    sf::FloatRect bounds = errorText.getLocalBounds();
    errorText.setOrigin(bounds.getCenter());

    // Устанавливаем позицию: центр по ширине, 10% отступа сверху
    // Ничего не сдвигаем, текст просто рисуется поверх
    errorText.setPosition(sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y * 0.1f));
}

void MainMenu::createLabel(float x, float y, const std::string& str, unsigned int size)
{
    sf::Text text(*resourceManager.getFont("main_font"), str);
    text.setCharacterSize(size);
    text.setFillColor(sf::Color(200, 200, 200));

    // Позиционируем по координатам экрана
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
    // Очищаем ошибку при обновлении меню (смене экрана)
    errorText.setString("");

    sf::Vector2u winSize = window.getSize();
    mainButtons.clear();
    setupButtons.clear();
    aboutButtons.clear();
    backButton.clear();
    inputBoxes.clear();
    labels.clear();

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

    // 3. TIME INPUTS
    const sf::Font* font = resourceManager.getFont("main_font");
    float customRow = row + 0.16f;
    float labelYOffset = 0.01f;

    float labelW = 0.15f;
    float inputW = 0.10f;

    float minLabelX = 0.15f;
    float minInputX = minLabelX + labelW;
    float incLabelX = 0.55f;
    float incInputX = incLabelX + labelW;

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

    createLabel(minLabelX, customRow + labelYOffset, "Custom Min:");
    createLabel(incLabelX, customRow + labelYOffset, "Inc (sec):");

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

    // 4. PRESETS
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

    timeBtn("1 + 0", 1, 0, 0.1f, row);
    timeBtn("3 + 0", 3, 0, 0.27f, row);
    timeBtn("3 + 2", 3, 2, 0.44f, row);
    timeBtn("5 + 0", 5, 0, 0.61f, row);
    timeBtn("5 + 3", 5, 3, 0.78f, row);

    row += 0.08f;
    timeBtn("10 + 0", 10, 0, 0.1f, row);
    timeBtn("10 + 5", 10, 5, 0.27f, row);
    timeBtn("15 + 10", 15, 10, 0.44f, row);
    timeBtn("30 + 0", 30, 0, 0.61f, row);
    timeBtn("30 + 20", 30, 20, 0.78f, row);

    inputBoxes.push_back(std::move(minInput));
    inputBoxes.push_back(std::move(incInput));

    // --- 5. IP INPUT (Если выбран Network) ---
    // Поле ввода размещаем ниже, чтобы не накладываться на время
    float nextRow = customRow + 0.10f;

    if (currentConfig.opponentType == OpponentType::Network)
    {
        float ipLabelX = 0.3f;
        float ipInputX = 0.45f;
        float ipInputW = 0.25f;

        createLabel(ipLabelX, nextRow + labelYOffset, "Server IP:");

        // limit 15 символов для IPv4
        auto ipInput = std::make_unique<InputBox>(
            sf::Vector2f(winSize.x * ipInputX, winSize.y * nextRow),
            sf::Vector2f(winSize.x * ipInputW, winSize.y * 0.06f),
            *font, currentConfig.serverIp, true, 15);

        ipInput->setOnChange([this](std::string val) {
            currentConfig.serverIp = val;
        });

        inputBoxes.push_back(std::move(ipInput));

        // Кнопку старт сдвигаем ниже, только если появилось поле IP
        nextRow += 0.1f;
    }
    else
    {
        // Если поля нет, кнопка будет чуть выше
        nextRow += 0.05f;
    }

    // START BUTTON
    // Позиция Y зависит от nextRow, чтобы не наехать на IP
    float startBtnY = (nextRow > 0.85f) ? 0.90f : 0.85f;
    if (startBtnY > 0.88f)
        startBtnY = 0.88f; // Ограничиваем, чтобы не улетела совсем вниз

    setupButtons.push_back(createBtn({ 0.35f, startBtnY }, { 0.3f, 0.1f }, "START GAME", [this]() {
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
        for (auto& label : labels)
            window.draw(label);

        // Рисуем сообщение об ошибке (сверху, поверх всего)
        if (!errorText.getString().isEmpty())
            window.draw(errorText);
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