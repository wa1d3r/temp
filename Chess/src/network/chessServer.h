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

    // Хранилище настроек матча
    int hostColorInt = 0; // 0 - White, 1 - Black
    int timeMinutes = 10;
    int incrementSeconds = 0;

public:
    ChessServer(unsigned short port);
    void run();

private:
    void handleNewConnection();
    void handleClientActivity();
    void processPacket(sf::TcpSocket& sender, sf::Packet& packet);
    void startGame();
    void relayPacketToOthers(sf::TcpSocket& sender, sf::Packet& packet);
};