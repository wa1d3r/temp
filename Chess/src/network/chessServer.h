#pragma once
#include "../core/move.h"
#include "PacketType.h"
#include <SFML/Network.hpp>
#include <iostream>
#include <memory>
#include <vector>

class ChessServer
{
    sf::TcpListener listener;
    sf::SocketSelector selector;
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;
    unsigned short port;

    int hostColorInt = 0; // 0 - White, 1 - Black
    int timeMinutes = 10;
    int incrementSeconds = 0;
    int gameTypeInt = 0;
    int seed = 0;

    // Храним выбранный режим игры для каждого клиента
    int clientGameTypes[2] = { 0, 0 };
    // Флаги, прислал ли клиент свой конфиг
    bool clientsReady[2] = { false, false };

public:
    ChessServer(unsigned short port);
    void run();

private:
    void handleNewConnection();
    void handleClientActivity();

    // Изменили тип возврата на bool (true - всё ок, false - критическое изменение, выход из цикла)
    bool processPacket(sf::TcpSocket& sender, sf::Packet& packet);

    void startGame();
    void abortGame();
    void relayPacketToOthers(sf::TcpSocket& sender, sf::Packet& packet);
    int getClientIndex(sf::TcpSocket& socket);
};