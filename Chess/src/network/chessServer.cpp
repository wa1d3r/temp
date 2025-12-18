#include "chessServer.h"

ChessServer::ChessServer(unsigned short port)
    : port(port)
{
    clientsReady[0] = false;
    clientsReady[1] = false;
}

void ChessServer::run()
{
    if (listener.listen(port) != sf::Socket::Status::Done)
    {
        std::cerr << "Error: Could not listen to port " << port << std::endl;
        return;
    }

    std::cout << "Server is listening on port " << port << "..." << std::endl;
    selector.add(listener);

    while (true)
    {
        if (selector.wait())
        {
            if (selector.isReady(listener))
            {
                handleNewConnection();
            }
            handleClientActivity();
        }
    }
}

int ChessServer::getClientIndex(sf::TcpSocket& socket)
{
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i].get() == &socket)
            return static_cast<int>(i);
    }
    return -1;
}

void ChessServer::handleNewConnection()
{
    if (clients.size() >= 2)
    {
        sf::TcpSocket temp;
        if (listener.accept(temp) == sf::Socket::Status::Done)
        {
            temp.disconnect();
        }
        return;
    }

    auto client = std::make_unique<sf::TcpSocket>();

    if (listener.accept(*client) == sf::Socket::Status::Done)
    {
        if (auto addr = client->getRemoteAddress())
            std::cout << "New client connected: " << addr->toString() << std::endl;
        else
            std::cout << "New client connected (unknown IP)" << std::endl;

        selector.add(*client);
        clients.push_back(std::move(client));

        clientsReady[clients.size() - 1] = false;

        std::cout << "Clients connected: " << clients.size() << "/2" << std::endl;

        if (clients.size() == 2)
        {
            std::cout << "Waiting for both players to send GameConfig..." << std::endl;
        }
    }
}

void ChessServer::abortGame()
{
    std::cout << "Game Modes mismatch! Aborting connection for both players." << std::endl;

    sf::Packet packet;
    packet << static_cast<int>(PacketType::Disconnect);

    for (auto& client : clients)
    {
        client->send(packet);
        selector.remove(*client);
        client->disconnect();
    }
    clients.clear();
    clientsReady[0] = false;
    clientsReady[1] = false;
}

void ChessServer::startGame()
{
    std::cout << "Match started! Applying host settings..." << std::endl;
    std::cout << "Settings: Host=" << (hostColorInt == 0 ? "White" : "Black")
              << ", Time=" << timeMinutes << "+" << incrementSeconds
              << ", Mode=" << gameTypeInt << std::endl;

    sf::Packet p1;
    p1 << static_cast<int>(PacketType::GameConfig)
       << hostColorInt
       << timeMinutes
       << incrementSeconds
       << gameTypeInt
       << seed;
    clients[0]->send(p1);

    sf::Packet p2;
    int guestColorInt = (hostColorInt == 0) ? 1 : 0;
    p2 << static_cast<int>(PacketType::GameConfig)
       << guestColorInt
       << timeMinutes
       << incrementSeconds
       << gameTypeInt
       << seed;
    clients[1]->send(p2);

    sf::Packet pStart;
    pStart << static_cast<int>(PacketType::StartGame);

    for (auto& client : clients)
        client->send(pStart);
}

void ChessServer::handleClientActivity()
{
    for (auto it = clients.begin(); it != clients.end();)
    {
        sf::TcpSocket& socket = **it;

        if (selector.isReady(socket))
        {
            sf::Packet packet;
            sf::Socket::Status status = socket.receive(packet);

            if (status == sf::Socket::Status::Done)
            {
                if (!processPacket(socket, packet))
                {
                    break;
                }
                ++it;
            }
            else if (status == sf::Socket::Status::Disconnected)
            {
                std::cout << "Client disconnected" << std::endl;
                selector.remove(socket);
                int idx = getClientIndex(socket);
                if (idx != -1)
                    clientsReady[idx] = false;
                it = clients.erase(it);
            }
            else
            {
                ++it;
            }
        }
        else
        {
            ++it;
        }
    }
}

bool ChessServer::processPacket(sf::TcpSocket& sender, sf::Packet& packet)
{
    int typeInt;
    if (packet >> typeInt)
    {
        PacketType type = static_cast<PacketType>(typeInt);

        if (type == PacketType::Move)
        {
            Move move;
            if (packet >> move)
            {
                sf::Packet relayPacket;
                relayPacket << static_cast<int>(PacketType::Move) << move;
                relayPacketToOthers(sender, relayPacket);
            }
        }
        else if (type == PacketType::GameOver)
        {
            sf::Packet relayPacket;
            relayPacket << static_cast<int>(PacketType::GameOver);
            relayPacketToOthers(sender, relayPacket);
        }
        else if (type == PacketType::GameConfig)
        {
            int c, t, i, g, s;
            if (packet >> c >> t >> i >> g >> s)
            {
                int idx = getClientIndex(sender);
                if (idx != -1)
                {
                    clientGameTypes[idx] = g;
                    clientsReady[idx] = true;
                    std::cout << "Client " << idx << " config received. Mode: " << g << std::endl;

                    if (idx == 0)
                    {
                        hostColorInt = c;
                        timeMinutes = t;
                        incrementSeconds = i;
                        gameTypeInt = g;
                        seed = s;
                        std::cout << "Host settings updated." << std::endl;
                    }
                }

                if (clients.size() == 2 && clientsReady[0] && clientsReady[1])
                {
                    if (clientGameTypes[0] == clientGameTypes[1])
                    {
                        startGame();
                    }
                    else
                    {
                        std::cerr << "Mode mismatch: Host(" << clientGameTypes[0]
                                  << ") vs Guest(" << clientGameTypes[1] << ")" << std::endl;
                        abortGame();
                        return false;
                    }
                }
            }
        }
    }
    return true;
}

void ChessServer::relayPacketToOthers(sf::TcpSocket& sender, sf::Packet& packet)
{
    for (auto& client : clients)
    {
        if (client.get() != &sender)
        {
            client->send(packet);
        }
    }
}