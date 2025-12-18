#include "core/Stockfish.h"
#include "core/board.h"
#include "game_controller.h"
#include "graphic/ResourceManager.h"
#include "graphic/sfml_graphics.h"
#include "menu/MainMenu.h"
#include "network/NetworkClient.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <memory>

int main()
{
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Chess");
    window.setFramerateLimit(25);

    ResourceManager resourceManager;

    // ... (Загрузка текстур и шрифтов осталась без изменений) ...
    resourceManager.loadFont("main_font", "assets/fonts/MoonlitFlow-Regular.otf");
    resourceManager.loadTexture("cell_white", "assets/textures/white_cell.png");
    resourceManager.loadTexture("cell_black", "assets/textures/black_cell.png");
    resourceManager.loadTexture("frame", "assets/textures/frame.png");
    resourceManager.loadTexture("point", "assets/textures/highlight.png");
    resourceManager.loadTexture("current_pos", "assets/textures/current_pos.png");
    resourceManager.loadTexture("last_pos", "assets/textures/last_pos.png");
    resourceManager.loadTexture("check", "assets/textures/check.png");
    resourceManager.loadTexture("background", "assets/textures/background.png");
    resourceManager.loadTexture("active_clock", "assets/textures/active_clock.png");
    resourceManager.loadTexture("disactive_clock", "assets/textures/disactive_clock.png");

    std::string colors[] = { "white", "black" };
    std::string types[] = { "pawn", "rook", "knight", "bishop", "queen", "king" };
    for (const auto& c : colors)
    {
        for (const auto& t : types)
        {
            resourceManager.loadTexture(t + "_" + c, "assets/textures/" + c + "_" + t + ".png");
        }
    }

    AppState appState = AppState::Menu;

    MainMenu mainMenu(window, resourceManager);

    std::unique_ptr<GameController> gameController = nullptr;
    std::shared_ptr<SFMLGraphics> graphics = nullptr;

    mainMenu.setOnExit([&window]() {
        window.close();
    });

    mainMenu.setOnStartGame([&](GameConfig config) {
        graphics = std::make_shared<SFMLGraphics>(window, resourceManager, config.playerColor);

        std::unique_ptr<INetworkInterface> network = nullptr;
        std::unique_ptr<Stockfish> stockfish = nullptr;

        if (config.opponentType == OpponentType::Network)
        {
            auto netClient = std::make_unique<NetworkClient>();

            // Получаем IP из конфига
            std::string ip = config.serverIp;
            if (ip.empty())
                ip = "127.0.0.1";
            else
            {
                ip.erase(std::remove(ip.begin(), ip.end(), ' '), ip.end());
                ip.erase(std::remove(ip.begin(), ip.end(), '\r'), ip.end());
            }

            std::cout << "Connecting to [" << ip << "]..." << std::endl;

            if (netClient->connect(ip, 53000))
            {
                std::cout << "Connected. Sending game config..." << std::endl;
                config.seed = static_cast<int>(time(nullptr));

                netClient->sendGameConfig(
                    config.playerColor,
                    config.timeMinutes,
                    config.incrementSeconds,
                    static_cast<int>(config.gameType),
                    config.seed);

                std::cout << "Waiting for second player..." << std::endl;

                Color finalColor = Color::White;
                int finalTime = 10;
                int finalInc = 0;
                int finalGameTypeInt = 0;
                int finalSeed = 0;

                // Ожидание ответа сервера
                if (netClient->waitForStart(finalColor, finalTime, finalInc, finalGameTypeInt, finalSeed))
                {
                    GameType finalGameType = static_cast<GameType>(finalGameTypeInt);

                    if (finalGameType != config.gameType)
                    {
                        std::cout << "Error: Game mode mismatch!" << std::endl;
                        // Ошибка на английском
                        mainMenu.setErrorMessage("Game mode mismatch!");
                        return;
                    }

                    config.playerColor = finalColor;
                    config.timeMinutes = finalTime;
                    config.incrementSeconds = finalInc;
                    config.seed = finalSeed;

                    network = std::move(netClient);
                    graphics = std::make_shared<SFMLGraphics>(window, resourceManager, config.playerColor);
                }
                else
                {
                    // Сообщение: сервер разорвал соединение
                    std::cout << "Connection failed or server disconnected." << std::endl;
                    mainMenu.setErrorMessage("Server disconnected");
                    return;
                }
            }
            else
            {
                // Сообщение: не удается подключиться
                std::cout << "Failed to connect." << std::endl;
                mainMenu.setErrorMessage("Connection failed");
                return;
            }
        }
        else if (config.opponentType == OpponentType::AI)
        {
            stockfish = std::make_unique<Stockfish>("stockfish.exe");
        }

        // ... (Создание игры и контроллера осталось без изменений) ...
        std::unique_ptr<GameMode> gameMode;
        if (config.gameType == GameType::Classic)
            gameMode = std::make_unique<Test>();
        else
            gameMode = std::make_unique<Fischer>(config.seed);

        auto board = std::make_unique<Board>(std::move(gameMode), config.timeMinutes * 60, config.incrementSeconds);

        gameController = std::make_unique<GameController>(
            std::move(board),
            graphics,
            std::move(network),
            std::move(stockfish),
            config.playerColor);

        gameController->setOnGameEnd(
            [&appState]() {
                appState = AppState::EndGame;
            });

        graphics->setOnClickCallback([&](Position pos) {
            if (gameController)
                gameController->onClick(pos.getX(), pos.getY());
        });

        appState = AppState::Game;
    });

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (event->is<sf::Event::Resized>())
            {
                sf::Vector2u new_size = event->getIf<sf::Event::Resized>()->size;
                sf::FloatRect visibleArea(sf::Vector2f(0, 0), sf::Vector2f(new_size));
                window.setView(sf::View(visibleArea));

                if (appState == AppState::Game && graphics)
                {
                    graphics->createBoardButtons();
                }
                else if (appState == AppState::Menu)
                {
                    mainMenu.resize();
                }
            }

            if (appState == AppState::Menu)
            {
                mainMenu.handleEvent(event);
            }
            else if (appState == AppState::Game && graphics)
            {
                graphics->handleEvent(event);
            }
        }

        if (appState == AppState::EndGame)
        {
            appState = AppState::Menu;
            gameController.reset();
            graphics.reset();
            mainMenu.resize();
        }

        window.clear();

        if (appState == AppState::Menu)
        {
            mainMenu.draw();
        }
        else if (appState == AppState::Game && gameController)
        {
            gameController->update();
        }

        window.display();
    }

    return 0;
}