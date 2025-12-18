#include "NetworkClient.h"
#include "PacketType.h"
#include <iostream>

NetworkClient::NetworkClient()
{
    socket.setBlocking(false);
}

NetworkClient::~NetworkClient()
{
    socket.disconnect();
}

bool NetworkClient::connect(const std::string& ip, unsigned short port)
{
    peerResignedFlag = false;
    socket.setBlocking(true);

    auto resolvedIp = sf::IpAddress::resolve(ip);
    if (resolvedIp)
    {
        if (socket.connect(*resolvedIp, port) == sf::Socket::Status::Done)
        {
            std::cout << "Connected to server " << ip << ":" << port << std::endl;
            connected = true;
            socket.setBlocking(false);
            return true;
        }
    }

    std::cerr << "Failed to connect to " << ip << ":" << port << std::endl;
    connected = false;
    return false;
}

void NetworkClient::sendGameConfig(Color color, int timeMinutes, int incrementSeconds)
{
    if (!connected)
        return;

    sf::Packet packet;
    int colorInt = (color == Color::White) ? 0 : 1;

    packet << static_cast<int>(PacketType::GameConfig)
           << colorInt
           << timeMinutes
           << incrementSeconds;

    socket.send(packet);
}

bool NetworkClient::waitForStart(Color& assignedColor, int& timeMinutes, int& incrementSeconds)
{
    if (!connected)
        return false;

    std::cout << "Waiting for game start (Blocking mode)..." << std::endl;
    socket.setBlocking(true);

    bool gameStarted = false;

    while (!gameStarted)
    {
        sf::Packet packet;
        sf::Socket::Status status = socket.receive(packet);

        if (status == sf::Socket::Status::Disconnected)
        {
            std::cout << "Server disconnected." << std::endl;
            return false;
        }

        if (status == sf::Socket::Status::Done)
        {
            int typeInt;
            if (packet >> typeInt)
            {
                PacketType type = static_cast<PacketType>(typeInt);

                if (type == PacketType::GameConfig)
                {
                    int colorInt;
                    if (packet >> colorInt >> timeMinutes >> incrementSeconds)
                    {
                        assignedColor = (colorInt == 0) ? Color::White : Color::Black;
                        std::cout << "Config received: "
                                  << (colorInt == 0 ? "White" : "Black")
                                  << ", " << timeMinutes << "m+" << incrementSeconds << "s"
                                  << std::endl;
                    }
                }
                else if (type == PacketType::StartGame)
                {
                    std::cout << "Game Started!" << std::endl;
                    gameStarted = true;
                }
            }
        }
    }

    socket.setBlocking(false);
    return true;
}

void NetworkClient::sendMove(const Move& move)
{
    if (!connected)
        return;

    sf::Packet packet;
    packet << static_cast<int>(PacketType::Move) << move;

    if (socket.send(packet) != sf::Socket::Status::Done)
    {
        std::cout << "Warning: Failed to send move packet." << std::endl;
    }
}

Move NetworkClient::receiveMove()
{
    if (!connected)
        return Move();

    sf::Packet packet;
    sf::Socket::Status status = socket.receive(packet);

    if (status == sf::Socket::Status::Done)
    {
        int typeInt;
        if (packet >> typeInt)
        {
            PacketType type = static_cast<PacketType>(typeInt);

            if (type == PacketType::Move)
            {
                Move move;
                if (packet >> move)
                    return move;
            }
            else if (type == PacketType::GameOver)
            {
                std::cout << "Received GameOver signal." << std::endl;
                peerResignedFlag = true;
            }
            else if (type == PacketType::Disconnect)
            {
                std::cout << "Opponent disconnected." << std::endl;
                connected = false;
            }
        }
    }
    else if (status == sf::Socket::Status::Disconnected)
    {
        connected = false;
    }

    return Move();
}

bool NetworkClient::isConnected()
{
    return connected;
}

void NetworkClient::sendGameOver()
{
    if (!connected)
        return;
    sf::Packet packet;
    packet << static_cast<int>(PacketType::GameOver);
    socket.send(packet);
}

bool NetworkClient::isPeerResigned()
{
    if (peerResignedFlag)
    {
        peerResignedFlag = false;
        return true;
    }
    return false;
}