#include "core/Stockfish.h"
#include "core/board.h"
#include "network/NetworkClient.h"
#include "game_controller.h"
#include "graphic/ResourceManager.h"
#include "graphic/sfml_graphics.h"
#include "menu/MainMenu.h"
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <memory>

int main()
{
    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Chess");
    window.setFramerateLimit(25);

    ResourceManager resourceManager;

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
        std::unique_ptr<GameMode> gameMode;
        if (config.gameType == GameType::Classic)
            gameMode = std::make_unique<Test>();
        else
            gameMode = std::make_unique<Fischer>();

        auto board = std::make_unique<Board>(std::move(gameMode), config.timeMinutes * 60, config.incrementSeconds);

        graphics = std::make_shared<SFMLGraphics>(window, resourceManager, config.playerColor);

        std::unique_ptr<INetworkInterface> network = nullptr;
        std::unique_ptr<Stockfish> stockfish = nullptr;

        if (config.opponentType == OpponentType::Network)
        {
            auto netClient = std::make_unique<NetworkClient>();

            std::string ip;
            std::cout << "Enter Server IP (default 127.0.0.1): ";

            if (std::cin.peek() == '\n')
                std::cin.ignore();

            std::getline(std::cin, ip);

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
                std::cout << "Connected. Waiting for second player..." << std::endl;

                Color myColor = Color::White;

                if (netClient->waitForStart(myColor))
                {
                    config.playerColor = myColor;
                    network = std::move(netClient);

                    graphics = std::make_shared<SFMLGraphics>(window, resourceManager, config.playerColor);

                    graphics->setOnClickCallback([&](Position pos) {});
                }
                else
                {
                    std::cout << "Connection failed or server disconnected." << std::endl;
                    return;
                }
            }
            else
            {
                std::cout << "Failed to connect." << std::endl;
                return;
            }
        }
        else if (config.opponentType == OpponentType::AI)
        {
            stockfish = std::make_unique<Stockfish>("stockfish.exe");
        }

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