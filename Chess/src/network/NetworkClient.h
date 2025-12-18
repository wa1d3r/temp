#pragma once
#include "../game_interfaces.h"
#include "PacketType.h"
#include "../core/move.h"
#include <SFML/Network.hpp>
#include <iostream>

class NetworkClient : public INetworkInterface
{
    sf::TcpSocket socket;
    bool connected = false;
    bool peerResignedFlag = false;

public:
    NetworkClient();
    virtual ~NetworkClient();

    bool connect(const std::string& ip, unsigned short port);

    void sendGameConfig(Color color, int timeMinutes, int incrementSeconds, int gameType, int seed);
    bool waitForStart(Color& assignedColor, int& timeMinutes, int& incrementSeconds, int& gameType, int& seed);

    void sendMove(const Move& move) override;
    Move receiveMove() override;
    bool isConnected() override;
    void sendGameOver() override;
    bool isPeerResigned() override;
    void disconnect();
};