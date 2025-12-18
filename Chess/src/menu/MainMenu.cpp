#include "MainMenu.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

MainMenu::MainMenu(sf::RenderWindow& win, ResourceManager& rm)
    : window(win)
    , resourceManager(rm)
    , currentScreen(MenuScreen::Main)
    , errorText(*rm.getFont("main_font"))
    , aboutText(*rm.getFont("main_font"))
    , currentAnim(AnimType::None)
    , idleTimer(4.0f)
    , animSprite1(*rm.getTexture("background")) // Временная текстура, будет заменена
    , animSprite2(*rm.getTexture("background"))
    , secondActorActive(false)
{
    // Настройка фона
    backgroundSprite = std::make_unique<sf::Sprite>(*resourceManager.getTexture("background"));

    // Настройка текста ошибок
    errorText.setFillColor(sf::Color::Red);
    errorText.setOutlineColor(sf::Color::Black);
    errorText.setOutlineThickness(2.0f);

    // Настройка текста "О нас"
    aboutText.setFillColor(sf::Color::White);
    aboutText.setOutlineColor(sf::Color::Black);
    aboutText.setOutlineThickness(2.0f);
    aboutText.setLineSpacing(1.2f);

    std::string info = "Chess Game\n\n"
                       "Game Modes:\n"
                       "- Classic Chess: Standard international rules.\n"
                       "- Fischer Chess (960): Randomized starting positions\n"
                       "  to test your creativity and strategy.\n\n"
                       "Features:\n"
                       "- Local Multiplayer\n"
                       "- Network Play\n"
                       "- Stockfish AI Integration\n\n"
                       "Developed using C++ and SFML library.\n\n"
                       "Authors: wa1der, kqldbl\n";

    aboutText.setString(info);
    srand(static_cast<unsigned int>(time(0)));

    // Первичная настройка размеров
    resize();
}

void MainMenu::resize()
{
    // Масштабирование фона под размер окна
    const sf::Texture& bgTex = backgroundSprite->getTexture();
    float scaleX = static_cast<float>(window.getSize().x) / bgTex.getSize().x;
    float scaleY = static_cast<float>(window.getSize().y) / bgTex.getSize().y;
    backgroundSprite->setScale(sf::Vector2f(scaleX, scaleY));

    // Адаптация размера шрифта
    unsigned int charSize = static_cast<unsigned int>(window.getSize().y * 0.025f);
    if (charSize < 16)
        charSize = 16;
    aboutText.setCharacterSize(charSize);
    errorText.setCharacterSize(charSize);

    // Центрирование текста "О программе"
    sf::FloatRect bounds = aboutText.getLocalBounds();
    aboutText.setOrigin(bounds.getCenter());
    aboutText.setPosition(sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f));

    // Пересоздание кнопок с новыми координатами
    initButtons();
}

void MainMenu::setErrorMessage(const std::string& message)
{
    errorText.setString(message);
    sf::FloatRect bounds = errorText.getLocalBounds();
    errorText.setOrigin(bounds.getCenter());
    errorText.setPosition(sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y * 0.1f));
}

// Вспомогательный метод для создания текстовых меток
void MainMenu::createLabel(float x, float y, const std::string& str, unsigned int size)
{
    sf::Text text(*resourceManager.getFont("main_font"), str);
    text.setCharacterSize(size);
    text.setFillColor(sf::Color(200, 200, 200));
    text.setPosition(sf::Vector2f(window.getSize().x * x, window.getSize().y * y));
    labels.push_back(text);
}

// Фабрика кнопок с автоматическим расчетом отступов
std::unique_ptr<Button> MainMenu::createBtn(sf::Vector2f pos, sf::Vector2f size,
    const std::string& text, std::function<void()> onClick, sf::Color color)
{
    const sf::Font* font = resourceManager.getFont("main_font");
    return std::make_unique<Button>(
        pos, size, window.getSize(), 0.005f,
        onClick, text, 24, font,
        color, sf::Color(200, 200, 255), sf::Color::Black, sf::Color::Black);
}

// Основной метод инициализации кнопок
void MainMenu::initButtons()
{
    // Очистка сообщения об ошибке при перерисовке интерфейса
    errorText.setString("");

    sf::Vector2u winSize = window.getSize();

    // Удаление старых элементов
    mainButtons.clear();
    setupButtons.clear();
    backButton.clear();
    inputBoxes.clear();
    labels.clear();

    // Параметры верстки
    float btnW = 0.3f;
    float btnH = 0.08f;
    float startY = 0.3f;
    float gap = 0.12f;

    // --- Экран: Главное меню ---

    // Кнопка выбора классических шахмат
    mainButtons.push_back(createBtn({ 0.35f, startY }, { btnW, btnH }, "Classic Chess", [this]() {
        currentConfig.gameType = GameType::Classic;
        currentScreen = MenuScreen::GameSetup;
        initButtons();
    }));

    // Кнопка выбора шахмат Фишера
    mainButtons.push_back(createBtn({ 0.35f, startY + gap }, { btnW, btnH }, "Fischer Chess", [this]() {
        currentConfig.gameType = GameType::Fischer;
        currentScreen = MenuScreen::GameSetup;
        initButtons(); 
    }));

    // Кнопка "О программе" (запускает анимации через таймер)
    mainButtons.push_back(createBtn({ 0.35f, startY + gap * 2 }, { btnW, btnH }, "About", [this]() {
        currentScreen = MenuScreen::About;
        idleTimer = 4.0f;
        currentAnim = AnimType::None;
    }));

    // Выход из игры
    mainButtons.push_back(createBtn({ 0.35f, startY + gap * 3 }, { btnW, btnH }, "Exit", [this]() {
        if (onExit) onExit(); }, sf::Color(255, 150, 150)));

    // Кнопка "Назад"
    backButton.push_back(createBtn({ 0.05f, 0.9f }, { 0.15f, 0.06f }, "< Back", [this]() {
        currentScreen = MenuScreen::Main;
        initButtons();
    }));

    // --- Экран: Настройка игры ---
    if (currentScreen == MenuScreen::GameSetup)
    {
        createSetupButtons(winSize);
    }
}

// Создание кнопок и полей для экрана настройки игры
void MainMenu::createSetupButtons(sf::Vector2u winSize)
{
    float col1 = 0.1f, col2 = 0.4f, col3 = 0.7f;
    float row = 0.15f;

    // Выбор типа оппонента
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

    // Выбор цвета
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

    // Настройка контроля времени
    const sf::Font* font = resourceManager.getFont("main_font");
    float customRow = row + 0.16f;
    float labelYOffset = 0.01f;
    float inputW = 0.10f;
    float minLabelX = 0.15f;
    float minInputX = minLabelX + 0.15f;
    float incLabelX = 0.55f;
    float incInputX = incLabelX + 0.15f;

    // Поля ввода произвольного времени
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

    // Обработчики изменения текста в полях
    pMinInput->setOnChange([this](std::string val) { try { currentConfig.timeMinutes = std::stof(val); } catch (...) {} });
    pIncInput->setOnChange([this](std::string val) { try { currentConfig.incrementSeconds = std::stof(val); } catch (...) {} });

    // Пресеты времени
    auto timeBtn = [&](std::string txt, float m, float inc, float x, float y) {
        bool selected = (std::abs(currentConfig.timeMinutes - m) < 0.01f && std::abs(currentConfig.incrementSeconds - inc) < 0.01f);
        setupButtons.push_back(createBtn({ x, y }, { 0.15f, 0.06f }, txt, [this, m, inc, pMinInput, pIncInput]() {
                currentConfig.timeMinutes = m; currentConfig.incrementSeconds = inc;
                std::stringstream ssMin, ssInc; ssMin << m; ssInc << inc;
                pMinInput->setString(ssMin.str()); pIncInput->setString(ssInc.str());
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

    // Поле ввода IP для сетевой игры
    float nextRow = customRow + 0.10f;
    if (currentConfig.opponentType == OpponentType::Network)
    {
        createLabel(0.3f, nextRow + labelYOffset, "Server IP:");
        auto ipInput = std::make_unique<InputBox>(
            sf::Vector2f(winSize.x * 0.45f, winSize.y * nextRow),
            sf::Vector2f(winSize.x * 0.25f, winSize.y * 0.06f),
            *font, currentConfig.serverIp, true, 15);
        ipInput->setOnChange([this](std::string val) { currentConfig.serverIp = val; });
        inputBoxes.push_back(std::move(ipInput));
        nextRow += 0.1f;
    }
    else
    {
        nextRow += 0.05f;
    }

    // Кнопка START GAME
    float startBtnY = (nextRow > 0.85f) ? 0.90f : 0.85f;
    if (startBtnY > 0.88f)
        startBtnY = 0.88f;

    setupButtons.push_back(createBtn({ 0.35f, startBtnY }, { 0.3f, 0.1f }, "START GAME", [this]() {
        if (onStartGame) {

            // Финализация конфига перед запуском
            if (currentConfig.isRandomColor) currentConfig.playerColor = (rand() % 2 == 0) ? Color::White : Color::Black;
            if (currentConfig.timeMinutes <= 0) currentConfig.timeMinutes = 1;
            onStartGame(currentConfig);
        } }, sf::Color(100, 255, 100)));
}

// Обновление анимации
void MainMenu::update()
{
    float dt = dtClock.restart().asSeconds();

    // Анимации работают только на экране About
    if (currentScreen != MenuScreen::About)
        return;

    // Если анимация не активна, ждем 5 секунд
    if (currentAnim == AnimType::None)
    {
        idleTimer += dt;
        if (idleTimer >= 5.0f)
        {
            startRandomAnimation();
            idleTimer = 0.0f;
        }
    }
    else
    {
        // Обновление текущей анимации
        switch (currentAnim)
        {
        case AnimType::KingChase:
            updateKingChase(dt);
            break;
        case AnimType::PawnPromotion:
            updatePawnPromotion(dt);
            break;
        case AnimType::KnightsLeap:
            updateKnightsLeap(dt);
            break;
        case AnimType::BishopGlide:
            updateBishopGlide(dt);
            break;
        }
    }
}

// Выбор и запуск случайной анимации
void MainMenu::startRandomAnimation()
{
    int r = rand() % 4;
    float w = static_cast<float>(window.getSize().x);
    float h = static_cast<float>(window.getSize().y);
    float targetSize = h * 0.10f;

    // Настройка спрайта
    auto setupSprite = [&](sf::Sprite& s, const std::string& name) {
        s.setTexture(*resourceManager.getTexture(name), true);
        float texH = static_cast<float>(s.getTexture().getSize().y);
        float scale = targetSize / texH;
        s.setScale(sf::Vector2f(scale, scale));
        s.setOrigin(sf::Vector2f(0.f, 0.f));
        s.setRotation(sf::degrees(0));
    };

    animStateTime = 0.0f;
    animFlag = false;
    secondActorActive = false;
    if (r == 0) // Погоня короля
    {
        currentAnim = AnimType::KingChase;
        setupSprite(animSprite1, "king_black");
        setupSprite(animSprite2, "rook_white");

        sf::FloatRect b = animSprite1.getLocalBounds();
        animSprite1.setOrigin(sf::Vector2f(b.size.x / 2.0f, b.size.y / 2.0f));

        animSprite1.setPosition(sf::Vector2f(-targetSize * 1.5f, h * 0.7f + targetSize / 2));

        animSprite2.setPosition(sf::Vector2f(-targetSize * 4.0f, h * 0.7f));
    }
    else if (r == 1) // Превращение пешки
    {
        currentAnim = AnimType::PawnPromotion;
        setupSprite(animSprite1, "pawn_white");
        animSprite1.setPosition(sf::Vector2f(w * 0.8f, h + targetSize));
    }
    else if (r == 2) // Прыжки коней
    {
        currentAnim = AnimType::KnightsLeap;
        setupSprite(animSprite1, "knight_white");
        setupSprite(animSprite2, "knight_black");

        float jumpDist = targetSize * 1.5f;
        float startX = -jumpDist * 3.0f;

        animSprite1.setPosition(sf::Vector2f(startX, h * 0.8f));
        animSprite2.setPosition(sf::Vector2f(startX + jumpDist, h * 0.8f));
    }
    else // Слон скользит по диагонали
    {
        currentAnim = AnimType::BishopGlide;
        std::string tName = (rand() % 2 == 0) ? "bishop_white" : "bishop_black";
        setupSprite(animSprite1, tName);
        animSprite1.setPosition(sf::Vector2f(-targetSize, -targetSize));
    }
}

// Анимация 1: Король убегает от ладьи
void MainMenu::updateKingChase(float dt)
{
    animStateTime += dt;
    float w = static_cast<float>(window.getSize().x);

    sf::Vector2f kPos = animSprite1.getPosition();
    float kWidth = animSprite1.getGlobalBounds().size.x;

    // Движение короля
    if (!animFlag)
    {
        float cycle = fmod(animStateTime, 0.8f);
        if (cycle < 0.2f)
        {
            kPos.x += 400.0f * dt;
        }
        animSprite1.setPosition(kPos);
    }

    // Активация ладьи
    if (!secondActorActive)
    {
        if (kPos.x > w / 2.0f)
        {
            secondActorActive = true;
            sf::Vector2f rStartPos = animSprite2.getPosition();
            if (rStartPos.x < -kWidth * 2.0f)
            {
                animSprite2.setPosition(sf::Vector2f(-kWidth * 2.0f, rStartPos.y));
            }
        }
    }
    else
    {
        // Движение ладьи
        sf::Vector2f rPos = animSprite2.getPosition();
        rPos.x += 600.0f * dt;
        animSprite2.setPosition(rPos);

        // Ладья догнала короля
        if (!animFlag && rPos.x + kWidth * 0.5f > kPos.x)
        {
            animFlag = true;
            animSprite1.setRotation(sf::degrees(90)); // Король упал
        }
    }

    // Конец анимации
    if (animSprite2.getPosition().x > w + kWidth)
    {
        currentAnim = AnimType::None;
    }
}

// Анимация 2: Пешка идет вверх и превращается в ферзя
void MainMenu::updatePawnPromotion(float dt)
{
    animStateTime += dt;
    float h = static_cast<float>(window.getSize().y);
    float w = static_cast<float>(window.getSize().x);
    float ph = animSprite1.getScale().y;
    sf::Vector2f pos = animSprite1.getPosition();
    float spriteH = animSprite1.getGlobalBounds().size.y;

    // Движение вверх
    if (!animFlag)
    {
        float cycle = fmod(animStateTime, 0.8f);
        if (cycle < 0.2f)
        {
            pos.y -= 400.0f * dt;
        }
        animSprite1.setPosition(pos);

        // Превращение
        if (pos.y < ph + 5)
        {
            animFlag = true;
            float targetSize = h * 0.10f;
            animSprite1.setTexture(*resourceManager.getTexture("queen_white"), true);
            float texH = animSprite1.getTexture().getSize().y;
            float scale = targetSize / texH;
            animSprite1.setScale(sf::Vector2f(scale, scale));
        }
    }
    else
    {
        // Движение за ферзя
        float speed = 400.0f;
        pos.y += speed * dt;
        pos.x -= speed * dt;
        animSprite1.setPosition(pos);

        if (pos.y > h || pos.x < 0)
        {
            currentAnim = AnimType::None;
        }
    }
}

// Анимация 3: Прыжки коней
void MainMenu::updateKnightsLeap(float dt)
{
    float w = static_cast<float>(window.getSize().x);
    float h = static_cast<float>(window.getSize().y);
    float spriteSize = animSprite1.getGlobalBounds().size.x;

    float jumpDuration = 0.8f;
    float jumpDist = spriteSize * 1.5f;
    float groundY = h * 0.8f;
    float jumpHeight = h * 0.15f;

    animStateTime += dt;

    // Опеделение прыгающего коня
    int stepIndex = static_cast<int>(animStateTime / jumpDuration);
    float t = fmod(animStateTime, jumpDuration) / jumpDuration;

    float startX = -jumpDist * 3.0f;

    sf::Vector2f p1, p2;

    // Прыгает белый конь
    if (stepIndex % 2 == 0)
    {
        float k2_posIndex = (float)(stepIndex + 1);
        p2.x = startX + k2_posIndex * jumpDist;
        p2.y = groundY;

        float k1_startIndex = (float)stepIndex;
        float startX_k1 = startX + k1_startIndex * jumpDist;
        float endX_k1 = startX_k1 + 2.0f * jumpDist;

        p1.x = startX_k1 + (endX_k1 - startX_k1) * t;
        p1.y = groundY - std::sin(t * 3.14159f) * jumpHeight;
    }
    // Прыгает черный конь
    else
    {
        float k1_posIndex = (float)(stepIndex + 1);
        p1.x = startX + k1_posIndex * jumpDist;
        p1.y = groundY;

        float k2_startIndex = (float)stepIndex;
        float startX_k2 = startX + k2_startIndex * jumpDist;
        float endX_k2 = startX_k2 + 2.0f * jumpDist;

        p2.x = startX_k2 + (endX_k2 - startX_k2) * t;
        p2.y = groundY - std::sin(t * 3.14159f) * jumpHeight;
    }

    animSprite1.setPosition(p1);
    animSprite2.setPosition(p2);

    if (p1.x > w && p2.x > w)
    {
        currentAnim = AnimType::None;
    }
}

// Анимация 4: Слон двигается по диагонали
void MainMenu::updateBishopGlide(float dt)
{
    float w = static_cast<float>(window.getSize().x);
    float h = static_cast<float>(window.getSize().y);
    float spriteW = animSprite1.getGlobalBounds().size.x;

    sf::Vector2f pos = animSprite1.getPosition();
    float speed = 300.0f;
    pos.x += speed * dt;
    pos.y += (speed * (h / w)) * dt;

    animSprite1.setPosition(pos);

    if (pos.x > w + spriteW)
    {
        currentAnim = AnimType::None;
    }
}

// Обработка событий SFML и передача их кнопкам
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
        for (auto& btn : backButton)
            btn->handleEvent(event, window);
    }
}

// Отрисовка всех элементов
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
        if (!errorText.getString().isEmpty())
            window.draw(errorText);
    }
    else if (currentScreen == MenuScreen::About)
    {
        if (currentAnim != AnimType::None)
        {
            if (currentAnim == AnimType::KnightsLeap || currentAnim == AnimType::KingChase)
            {
                window.draw(animSprite1);
                window.draw(animSprite2);
            }
            else
            {
                window.draw(animSprite1);
            }
        }

        window.draw(aboutText);
        for (auto& btn : backButton)
            btn->draw(window);
    }
}