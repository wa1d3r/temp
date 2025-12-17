#pragma once
#include "PacketType.h"
#include <SFML/Network.hpp>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "../core/move.h"

class ChessServer
{
public:
    explicit ChessServer(unsigned short port);
    void run();

private:
    void handleNewConnection();
    void handleClientActivity();

    void processPacket(sf::TcpSocket& sender, sf::Packet& packet);

    void sendPacket(sf::TcpSocket& socket, PacketType type, const std::string& msg = "");
    void sendPacket(sf::TcpSocket& socket, PacketType type, int data);

    void relayPacketToOthers(sf::TcpSocket& sender, sf::Packet& packet);
    void broadcastPacket(PacketType type, const std::string& msg);

    void startGame();
    void handleDisconnect(sf::TcpSocket& disconnectedSocket);

    unsigned short port;
    sf::TcpListener listener;
    sf::SocketSelector selector;

    std::vector<std::unique_ptr<sf::TcpSocket>> clients;

    bool gameStarted = false;
};